#include "util/smstext.h"
#include "SmscConnector.h"
#include "TrafficControl.hpp"
#include "TaskProcessor.h"
#include "core/buffers/JStore.hpp"
// #include "util/config/region/RegionFinder.hpp"

namespace {

using namespace smsc::infosme;
using namespace smsc::util::config;

std::string cgetString( ConfigView& cv, const char* tag, const char* what )
{
    std::auto_ptr<char> str(cv.getString(tag,what));
    return std::string(str.get());
}

inline size_t WriteKey(File& f, const ReceiptId& key)
{
    return key.Write(f);
}

inline size_t ReadKey(File& f, ReceiptId& key)
{
    return key.Read(f);
}

inline size_t WriteRecord( smsc::core::buffers::File& f,
                           const ReceiptId& key,
                           const TaskMsgId& val )
{
    uint8_t keySize = uint8_t(WriteKey(f, key));
    f.WriteNetInt64(val.msgId);
    f.WriteNetInt32(val.taskId);
    return keySize+8+4; 
}

inline size_t ReadRecord( smsc::core::buffers::File& f,
                          ReceiptId& key,
                          TaskMsgId& val )
{
    uint8_t keySize = uint8_t(ReadKey(f, key));
    val.msgId=f.ReadNetInt64();
    val.taskId=f.ReadNetInt32();
    return keySize+8+4;
}

class MessageGuard
{
    typedef enum { msgProcessed, msgSuspended, msgFailed } MessageState;
public:
    MessageGuard(Task& argTask, const Message& theMsg) :
    task(argTask), msg(theMsg), state(msgSuspended),
    smppState(smsc::system::Status::SYSERR)
    {
    }

    ~MessageGuard()
    {
        /* if (state == msgResending ) {
            TaskProcessor::retryMessage(&task,msg.id);
        } else */
        if (state == msgFailed ) {
            task.finalizeMessage(msg.id,FAILED,smppState);
        } else if ( state == msgSuspended ) {
            task.putToSuspendedMessagesQueue(msg);
        }
    }

    void processed() {
        state = msgProcessed;
    }
    void suspended( int smpp ) {
        state = msgSuspended;
        smppState = smpp;
    }
    void failed(int smpp) {
        state = msgFailed;
        smppState = smpp;
    }

    bool isProcessed() const { return state == msgProcessed; }
    bool isFailed() const { return state == msgFailed; }

private:
    Task& task;
    const Message& msg;
    MessageState state;
    int          smppState;
};

} // anonymous namespace


namespace smsc {
namespace infosme {

using smsc::core::synchronization::MutexGuard;

class SmscConnector::EventRunner : public smsc::core::threads::ThreadedTask
{
public:
    EventRunner( EventMethod method, SmscConnector& proc, const ResponseData& argRd ) :
    method(method), processor(proc), rd(argRd) {
        ++processor.usage_; // externally locked
    }
    
    virtual ~EventRunner() {
        MutexGuard mg(processor.destroyMonitor_);
        if ( --processor.usage_ == 0 ) processor.destroyMonitor_.notifyAll();
    }
    
    virtual int Execute()
    {
        switch (method)
        {
        case processResponseMethod:
            processor.processResponse(rd);
            if (!rd.trafficst) TrafficControl::incIncoming();
            break;
        case processReceiptMethod:
            processor.processReceipt(rd);
            break;
        default:
            __trace2__("Invalid method '%d' invoked by event.", method);
            return -1;
        }
        return 0;
    };
    virtual const char* taskName() {
        return "InfoSmeEvent";
    };
private:
    EventMethod    method;
    SmscConnector& processor;
    ResponseData   rd;
};


struct SmscConnector::JStoreWrapper 
{
    smsc::core::buffers::JStore< ReceiptId, TaskMsgId, ReceiptId > jstore;
    JStoreWrapper( const std::string& storeLocation,
                   const std::string& smscId,
                   int mappingRollTime,
                   size_t mappingMaxChanges ) {
        const std::string jLocation = storeLocation + "jstore";
        if ( ! File::IsDir(jLocation.c_str()) ) {
            // trying to unlink first
            try {
                File::Unlink(jLocation.c_str());
            } catch (...) {}
            File::MkDir(jLocation.c_str());
        }
        const std::string path = jLocation + "/mapping" + smscId + ".bin";
        jstore.Init( path.c_str(), mappingRollTime, mappingMaxChanges );
    }
    ~JStoreWrapper() {
        jstore.Stop();
    }

private:
    JStoreWrapper();
};

// ===================================================================

smsc::sme::SmeConfig SmscConnector::readSmeConfig( ConfigView& config )
{
    {
        smsc::logger::Logger* tmplog = smsc::logger::Logger::getInstance("is2.conn");
        smsc_log_debug(tmplog,"reading sme config");
    }

    smsc::sme::SmeConfig rv;
    // Mandatory fields
    rv.host = ::cgetString(config,"host", "SMSC host wasn't defined !");
    rv.sid = ::cgetString(config,"sid", "InfoSme id wasn't defined !");
    rv.port = config.getInt("port", "SMSC port wasn't defined !");
    rv.timeOut = config.getInt("timeout", "Connect timeout wasn't defined !");

    // Optional fields
    try {
        rv.password = ::cgetString(config,"password","InfoSme password wasn't defined !");
    } catch (smsc::util::config::ConfigException&) {}
    try {
        const std::string systemType = ::cgetString(config,"systemType","InfoSme system type wasn't defined !");
        rv.setSystemType(systemType);
    } catch (smsc::util::config::ConfigException&) {}
    /*
    try {
        rv.origAddr = ::cgetString(config,"origAddress",
                                   "InfoSme originating address wasn't defined !");
    } catch (smsc::util::config::ConfigException&) {}
     */
    try {
        rv.interfaceVersion = config.getInt("interfaceVersion","InfoSme interface version wasn't defined!");
    } catch (smsc::util::config::ConfigException&) {}
    try {
        const std::string ar = ::cgetString(config,"rangeOfAddress","InfoSme range of address was not defined");
        rv.setAddressRange(ar);
    } catch (smsc::util::config::ConfigException&) {}
    return rv;
}


SmscConnector::SmscConnector( TaskProcessor& processor,
                              const string& smscId,
                              bool doPerformanceTests ):
smscId_(smscId),
log_(Logger::getInstance("smsc.infosme.connector")),
processor_(processor),
timeout_(10),
stopped_(false),
connected_(false),
listener_(*this, log_),
session_(0),
// new SmppSession(cfg, &listener_)),0),
usage_(0),
jstore_(0)
#ifdef INTHASH_USAGE_CHECKING
,taskIdsBySeqNum(SMSCFILELINE)
#endif
{
    listener_.setSyncTransmitter(session_->getSyncTransmitter());
    listener_.setAsyncTransmitter(session_->getAsyncTransmitter());
    if (doPerformanceTests) {
        performanceTester_.reset(new PerformanceTester(smscId_,listener_));
        listener_.setPerformanceTester(performanceTester_.get());
    }
    jstore_ = new JStoreWrapper(processor_.getStoreLocation(),
                                smscId_,
                                processor_.getMappingRollTime(),
                                processor_.getMappingMaxChanges() );
    // we have to restore all timers
    const time_t expireTime = time(0) + processor_.getReceiptWaitTime();
    ReceiptId key;
    TaskMsgId value;
    MutexGuard recptGuard(receiptWaitQueueLock);
    unsigned count = 0;
    for ( jstore_->jstore.First(); jstore_->jstore.Next(key,value); ) {
        receiptWaitQueue.Push(ReceiptTimer(expireTime,key));
        smsc_log_debug(log_,"restoring receipt timer for %s",key);
        ++count;
    }
    smsc_log_info(log_,"smsc %s: %u receipt timers restored",smscId.c_str(),count);
}


SmscConnector::~SmscConnector()
{
    stop();
    WaitFor();
    // waiting until all dependent objects finished
    while ( true ) {
        MutexGuard mg(destroyMonitor_);
        if ( usage_ <= 0 ) break;
        destroyMonitor_.wait(100);
    }
    clearHashes();
    if ( jstore_ ) { delete jstore_; }
}

void SmscConnector::start()
{
    MutexGuard mg(stateMonitor_);
    stopped_ = false;
    if (performanceTester_.get()) performanceTester_->start();
    Start();
}

void SmscConnector::stop() {
    MutexGuard mg(stateMonitor_);
    stopped_ = true;
    if (performanceTester_.get()) performanceTester_->stop();
    stateMonitor_.notify();
}

void SmscConnector::reconnect() {
    MutexGuard mg(stateMonitor_);
    connected_ = false;
    stateMonitor_.notify();
}

void SmscConnector::updateConfig( const smsc::sme::SmeConfig& config )
{
    smsc_log_warn(log_, "updateConfig on '%s'... ", smscId_.c_str());
    {
        MutexGuard mg(destroyMonitor_);
        timeout_ = config.timeOut;
        if (session_.get()) session_->close();
        std::auto_ptr<SmppSession> newsess(new SmppSession(config,&listener_));
        listener_.setSyncTransmitter(newsess->getSyncTransmitter());
        listener_.setAsyncTransmitter(newsess->getAsyncTransmitter());
        // std::auto_ptr<SmppSession> oldsess(session_.release());
        session_.reset(newsess.release());
        destroyMonitor_.notifyAll();
    }
    {
        MutexGuard mg(stateMonitor_);
        connected_ = false;
        stateMonitor_.notify();
    }
}

bool SmscConnector::isStopped() const {
    return stopped_;
}

int SmscConnector::Execute() { 

    while (!isStopped()) {
        // after call to isNeedStop() was completed all signals is locked.
        // any thread being started from this point has signal mask with all signals locked 
        smsc_log_info(log_, "Connecting to SMSC id='%s'... ", smscId_.c_str());
        try
        {
            {
                MutexGuard mg(destroyMonitor_);
                session_->close(); // make sure the session is stopped
                clearHashes();
                // session_->connect();
                if ( performanceTester_.get() ) {
                    smsc_log_info(log_,"Skipping session connection for performance test");
                    performanceTester_->connect();
                } else {
                    session_->connect();
                }
            }

            {
                MutexGuard mg(stateMonitor_);
                connected_ = true;
                while ( connected_ && !isStopped() ) {
                    stateMonitor_.wait();
                }
                if ( !isStopped() ) {
                    smsc_log_info(log_, "need reconnect to SMSC id='%s'.", smscId_.c_str());
                }
            }

        } catch (SmppConnectException& exc) {

            const char* msg = exc.what();
            smsc_log_error(log_, "Connect to SMSC id='%s' failed. Cause: %s", smscId_.c_str(), (msg) ? msg:"unknown");
            {
                MutexGuard mg(stateMonitor_);
                connected_ = false;
                if (exc.getReason() == SmppConnectException::Reason::bindFailed) {
                    stopped_ = true;
                    smsc_log_error(log_, "SMSC Connector id='%s' disabled!", smscId_.c_str());
                    break;
                }
                smsc_log_debug(log_,"going to sleep for %u seconds",timeout_);
                stateMonitor_.wait(timeout_*1000);
            }
        } // if exception occured
    } // while is not stopped
    smsc_log_info(log_, "SMSC Connector '%s' stopped", smscId_.c_str());
    return 1; 
}

int SmscConnector::getSeqNum() {
    MutexGuard guard(destroyMonitor_);
    return session_->getNextSeq();
}


uint32_t SmscConnector::sendSms(const std::string& org,const std::string& dst,const std::string& txt,bool flash) {
  smsc_log_info(log_, "send test sms to:'%s' from:'%s' text:'%s', SMSC id: '%s'", dst.c_str(), org.c_str(), txt.c_str(), smscId_.c_str());
  if (!connected_) {
      smsc_log_warn(log_, "SMSC Connector '%s' is not connected.", smscId_.c_str());
      return SmppStatusSet::ESME_RUNKNOWNERR;
  }

  PduSubmitSm sbm;
  PduPartSm& msg=sbm.get_message();
  msg.set_source(smsc::smpp::Address2PduAddress(org.c_str()));
  msg.set_dest(smsc::smpp::Address2PduAddress(dst.c_str()));
  msg.set_esmClass(0x02);//forward mode
  if(flash)
  {
    sbm.get_optional().set_destAddrSubunit(1);
  }

  if(smsc::util::hasHighBit(txt.c_str(),txt.length()))
  {
    msg.set_dataCoding(DataCoding::UCS2);
    TmpBuf<short,1024> tmp(txt.length()+1);
    int len=ConvertMultibyteToUCS2(txt.c_str(), txt.length(), tmp.get(), txt.length()*2+2,
                                   CONV_ENCODING_CP1251);
    for(int i=0;i<len;i++)
    {
      tmp.get()[i]=htons(tmp.get()[i]);
    }        
    sbm.get_optional().set_messagePayload((char*)tmp.get(),len);
  }else
  {
    msg.set_dataCoding(DataCoding::LATIN1);
    sbm.get_optional().set_messagePayload(txt.c_str(),int(txt.length()));
  }
  sbm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
  sbm.get_header().set_sequenceNumber(getSeqNum());
  PduSubmitSmResp* resp=session_->getSyncTransmitter()->submit(sbm);
  if(!resp)
  {
    return SmppStatusSet::ESME_RUNKNOWNERR;
  }
  uint32_t rv=resp->get_header().get_commandStatus();
  disposePdu((SmppHeader*)resp);
  return rv;
}


bool SmscConnector::invokeProcessReceipt( const ResponseData& rd )
{
    EventRunner* er = 0;
    {
        if ( stopped_ ) return false;
        MutexGuard mg(destroyMonitor_);
        if ( stopped_ ) return false; // NOTE: we intentionally lock by different monitor
        er = new EventRunner(processReceiptMethod,*this,rd);
    }
    return processor_.invokeProcessEvent(er);
}


bool SmscConnector::invokeProcessResponse( const ResponseData& rd )
{
    EventRunner* er = 0;
    {
        if ( stopped_ ) return false;
        MutexGuard mg(destroyMonitor_);
        if ( stopped_ ) return false;
        er = new EventRunner(processResponseMethod,*this,rd);
    }
    return processor_.invokeProcessEvent(er);
}


int SmscConnector::send( Task& task, Message& msg )
{
    char whatbuf[100];
    const char* what = "";
    int seqNum = 0;
    MessageGuard msguard(task,msg);
    // msguard.suspended( smsc::system::Status::SYSERR );

    MutexGuard mg(destroyMonitor_);
    smsc_log_debug(log_,"smsc '%s': send msg %llx of task %u/'%s'",
                   smscId_.c_str(), msg.id, task.getId(), task.getName().c_str() );

    unsigned nchunks = 1;
    do {
        if ( stopped_ ) {
            what = "stopped";
            break;
        } else if ( !session_.get() ) {
            what = "no session";
            break;
        } else if ( !connected_ ) {
            // session_->isClosed() ) {
            what = "not connected";
            break;
        } else if ( ! session_->getAsyncTransmitter() ) {
            what = "session has no transmitter";
            break;
        }

        seqNum = session_->getNextSeq();
        {
            MutexGuard snGuard(taskIdsBySeqNumMonitor);
            const int seqNumsCount = taskIdsBySeqNum.Count();
            if ( seqNumsCount > processor_.getUnrespondedMessagesMax() ) {
                what = "too many msgs in queue";
                break;
            }
            if ( taskIdsBySeqNum.Exist(seqNum) ) {
                smsc_log_warn(log_, "smsc '%s': seqnum=%d replaced for msg %llx, task %u/'%s'",
                              smscId_.c_str(), seqNum, msg.id, task.getId(), task.getName().c_str() );
                taskIdsBySeqNum.Delete(seqNum);
            }
            taskIdsBySeqNum.Insert(seqNum, TaskMsgId(task.getId(), msg.id));
        }

        const time_t now = time(NULL);
        {
            MutexGuard respGuard(responseWaitQueueLock);
            responseWaitQueue.Push(ResponseTimer(now+processor_.getResponseWaitTime(), seqNum));
        }

        try {

            const TaskInfo& info(task.getInfo());
        
            smsc::sms::Address oa, da;
            const std::string& oaStr = 
                info.address.empty() ? processor_.getAddress() : info.address;
            if ( !info.convertMSISDNStringToAddress(oaStr.c_str(),oa) ) {
                if (log_->isErrorEnabled()) {
                    sprintf(whatbuf,"invalid oa: %s",oaStr.c_str());
                    what = whatbuf;
                }
                msguard.failed(smsc::system::Status::INVSRCADR);
                break;
            }
            if ( !info.convertMSISDNStringToAddress(msg.abonent.c_str(),da) ) {
                if (log_->isErrorEnabled()) {
                    sprintf(whatbuf,"invalid da: %s",msg.abonent.c_str());
                    what = whatbuf;
                }
                msguard.failed(smsc::system::Status::INVDSTADR);
                break;
            }

            smsc::sms::SMS sms;
            sms.setOriginatingAddress(oa);
            sms.setDestinationAddress(da);
            sms.setArchivationRequested(false);
            sms.setDeliveryReport(1);
            sms.setValidTime( (info.validityDate <= 0 || info.validityPeriod > 0) ?
                              now+info.validityPeriod : info.validityDate );
        
            sms.setIntProperty(smsc::sms::Tag::SMPP_REPLACE_IF_PRESENT_FLAG,
                               (info.replaceIfPresent) ? 1:0);
            sms.setEServiceType( (info.replaceIfPresent && info.svcType.length()>0) ?
                                 info.svcType.c_str() : processor_.getSvcType() );
    
            sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID, processor_.getProtocolId());
            sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS, (info.transactionMode) ? 2:0);
            sms.setIntProperty(smsc::sms::Tag::SMPP_PRIORITY, 0);
            sms.setIntProperty(smsc::sms::Tag::SMPP_REGISTRED_DELIVERY, 1);

            if ( info.flash ) {
                sms.setIntProperty(smsc::sms::Tag::SMPP_DEST_ADDR_SUBUNIT,1);
            }

            const char* out = msg.message.c_str();
            size_t outLen = msg.message.length();
            char* msgBuf = 0;
            if (smsc::util::hasHighBit(out,outLen)) {
                size_t msgLen = outLen*2;
                msgBuf = new char[msgLen];
                ConvertMultibyteToUCS2(out, outLen, (short*)msgBuf, msgLen,
                                       CONV_ENCODING_CP1251);
                sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, DataCoding::UCS2);
                out = msgBuf;
                outLen = msgLen;
            } else {
                sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, DataCoding::LATIN1);
            }

            try {
                if (outLen <= MAX_ALLOWED_MESSAGE_LENGTH && !info.useDataSm) {
                    sms.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, out, (unsigned)outLen);
                    sms.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, (unsigned)outLen);
                } else if ( info.useUssdPush ) {
                    if (outLen > MAX_ALLOWED_MESSAGE_LENGTH ) {
                        smsc_log_warn(log_,"ussdpush: max allowed msg length reached: %u",unsigned(outLen));
                        outLen = MAX_ALLOWED_MESSAGE_LENGTH;
                    }
                    sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, out, (unsigned)outLen);
                    sms.setIntProperty(Tag::SMPP_SM_LENGTH, (unsigned)outLen);
                } else {
                    if (outLen > MAX_ALLOWED_PAYLOAD_LENGTH) {
                        outLen = MAX_ALLOWED_PAYLOAD_LENGTH;
                    }
                    sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, out, (unsigned)outLen);
                }
            } catch (...) {
                if (msgBuf) delete msgBuf; msgBuf = 0;
                msguard.failed(smsc::system::Status::SYSERR);
                what = "wrong message body (set/get failed)";
                break;
            }
            if (msgBuf) delete msgBuf;

            if (info.useUssdPush) {
                try {
                    sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP,smsc::smpp::UssdServiceOpValue::USSN_REQUEST);
                } catch (...) {
                    smsc_log_error(log_,"ussdpush: cannot set USSN_REQ");
                    msguard.failed(smsc::system::Status::SYSERR);
                    return 0;
                }
            }

            const int chunkLen = processor_.getMaxMessageChunkSize();
            if (chunkLen > 0 && outLen > chunkLen ) {
                // SMS will be splitted into nchunks chunks (estimation)
                nchunks = (outLen-1) / chunkLen + 1;
            }

            if (info.useDataSm) {
                smsc_log_debug(log_, "Send DATA_SM");
                uint32_t validityDate = info.validityDate <= now ? 0 : static_cast<uint32_t>(info.validityDate - now);
                sms.setIntProperty(smsc::sms::Tag::SMPP_QOS_TIME_TO_LIVE, (info.validityDate <= 0 || info.validityPeriod > 0) ?
                                   static_cast<uint32_t>(info.validityPeriod) : validityDate);
                PduDataSm dataSm;
                dataSm.get_header().set_sequenceNumber(seqNum);
                dataSm.get_header().set_commandId(SmppCommandSet::DATA_SM);
                fillDataSmFromSms(&dataSm, &sms);
                // session_->getAsyncTransmitter()->sendPdu(&(dataSm.get_header()));
                if (performanceTester_.get()) {
                    smsc_log_debug(log_,"skipping send data_sm for perftest");
                    performanceTester_->sendPdu( &(dataSm.get_header()));
                } else {
                    session_->getAsyncTransmitter()->sendPdu(&(dataSm.get_header()));
                }
            } else {
                smsc_log_debug(log_, "Send SUBMIT_SM");
                PduSubmitSm submitSm;
                submitSm.get_header().set_sequenceNumber(seqNum);
                submitSm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
                fillSmppPduFromSms(&submitSm, &sms);
                // session_->getAsyncTransmitter()->sendPdu(&(submitSm.get_header()));
                if (performanceTester_.get()) {
                    smsc_log_debug(log_,"skipping send submit_sm for perftest");
                    performanceTester_->sendPdu(&(submitSm.get_header()));
                } else {
                    session_->getAsyncTransmitter()->sendPdu(&(submitSm.get_header()));
                }
            }
            TrafficControl::incOutgoing(nchunks);

            // success
            msguard.processed();
            break;

        } catch ( std::exception& e ) {
            if ( log_->isErrorEnabled() ) {
                snprintf(whatbuf,sizeof(whatbuf),"exc: %s",e.what());
                what = whatbuf;
            }
        }

    } while ( false );

    if ( msguard.isProcessed() ) {
        smsc_log_debug(log_,"smsc '%s': msg %llx, abnt %s, task %u/'%s' sent",
                       smscId_.c_str(), msg.id, msg.abonent.c_str(), task.getId(), task.getName().c_str() );
        return nchunks;
    }

    if ( seqNum != 0 ) {
        MutexGuard snGuard(taskIdsBySeqNumMonitor);
        if ( taskIdsBySeqNum.Delete(seqNum) ) {
            taskIdsBySeqNumMonitor.notifyAll();
        }
    }
    smsc_log_error(log_,"smsc '%s': %s msg %llx, abnt %s, task %u/'%s': %s", smscId_.c_str(),
                   msguard.isFailed() ? "failed" : "suspended",
                   msg.id, msg.abonent.c_str(), task.getId(), task.getName().c_str(), what );
    return 0;
}


void SmscConnector::processWaitingEvents( time_t tm )
{
    int count = 0;
    // process responses
    do {
        ResponseTimer timer;
        {
            MutexGuard respGuard(responseWaitQueueLock);
            count = responseWaitQueue.Count();
            if ( count <= 0 ) { break; } // no events
            if ( responseWaitQueue[0].timer > tm ) { break; } // too early
            responseWaitQueue.Shift(timer);
        }
        bool needProcess = false;
        {
            MutexGuard mg(taskIdsBySeqNumMonitor);
            needProcess = taskIdsBySeqNum.Exist(timer.seqNum);
        }
        if (needProcess) {
            ResponseData rd(smsc::system::Status::MSGQFUL,timer.seqNum,"");
            processResponse(rd, true);
        }
    } while ( ! processor_.bNeedExit );
        
    // process receipts
    do {
        ReceiptTimer timer;
        {
            MutexGuard recptGuard(receiptWaitQueueLock);
            count = receiptWaitQueue.Count();
            if ( count <= 0 ) { break; }
            if ( receiptWaitQueue[0].timer > tm ) { break; }
            receiptWaitQueue.Shift(timer);
        }

        bool needProcess = false;
        {
            MutexGuard guard(receiptsLock);
            ReceiptData* receiptPtr = receipts.GetPtr(timer.receiptId);
            if (receiptPtr) { 
                smsc_log_warn(log_, "%s for smscMsgId='%s' smscConnectorId='%s' wasn't received and timed out!",
                              ((receiptPtr->receipted) ? "Receipt": "Response"),
                              timer.receiptId.getMessageId(), smscId_.c_str());
                needProcess = true;
            }
        }
        if (needProcess)
        {
            processReceipt(ResponseData(smsc::system::Status::MSGQFUL,0,timer.receiptId.getMessageId()), true);
        }

    } while (!processor_.bNeedExit);
}


void SmscConnector::processResponse( const ResponseData& rd, bool internal )
{
    if (!internal) {
        smsc_log_info(log_, "Response(%s): seqNum=%d, smscMsgId=%s, accepted=%d, retry=%d, immediate=%d",
                      smscId_.c_str(),
                      rd.seqNum, rd.msgId.c_str(),rd.accepted, rd.retry, rd.immediate);
    } else {
        smsc_log_info(log_, "Response(%s) for seqNum=%d is timed out.",
                      smscId_.c_str(), rd.seqNum);
    }

    TaskMsgId tmIds;
    {
        TaskMsgId* tmIdsPtr = 0;
        MutexGuard snGuard(taskIdsBySeqNumMonitor);
        if (!(tmIdsPtr = taskIdsBySeqNum.GetPtr(rd.seqNum)))
        {
            if (!internal)
            {
                smsc_log_warn(log_, "Response(%s): Sequence number=%d is unknown!",
                              smscId_.c_str(), rd.seqNum);
            }
            return;
        }
        tmIds = *tmIdsPtr;
        taskIdsBySeqNum.Delete(rd.seqNum);
        taskIdsBySeqNumMonitor.notifyAll();
    }

    try
    {
        ReceiptId receiptId(rd.msgId);
        ReceiptData receipt; // receipt.receipted = false

        bool receiptInCache = ( !internal && rd.accepted );

        if ( receiptInCache ) {
            MutexGuard guard(receiptsLock);
            ReceiptData* receiptPtr = receipts.GetPtr(receiptId);
            if (receiptPtr) {
                receipt = *receiptPtr;
                receiptInCache = false;
            } else {
                receipts.Insert(receiptId, receipt);
                MutexGuard recptGuard(receiptWaitQueueLock);
                receiptWaitQueue.Push(ReceiptTimer(time(NULL)+processor_.getReceiptWaitTime(), receiptId));
            }
        }

        // check what processor tells us about receipts
        if ( ! processor_.processResponse(tmIds,rd,internal,receipt.receipted) ) {
            if ( receiptInCache ) {
                // delete newly created receipt in cache
                MutexGuard guard(receiptsLock);
                receipts.Delete(receiptId);
            }
            return;
        }

        bool idMappingCreated = false;
        if ( !receipt.receipted ) {
            smsc_log_debug(log_, "Response(%s): Receipt ID: msgId='%s'",
                           smscId_.c_str(), receiptId.getMessageId());
            jstore_->jstore.Insert(receiptId, tmIds);
            idMappingCreated = true;
        }

        {
            MutexGuard guard(receiptsLock);
            ReceiptData* receiptPtr = receipts.GetPtr(receiptId);
            if (receiptPtr) {
                receipt = *receiptPtr;
                receipts.Delete(receiptId);
            }
            else receipt.receipted = false;
        }

        if (receipt.receipted) // receipt already come
        {
            smsc_log_debug(log_, "Response(%s): receipt come when response is in process",
                           smscId_.c_str() );
            if (idMappingCreated)
            {
                jstore_->jstore.Delete(receiptId);
            }
            ResponseData rd2(rd);
            rd2.accepted=receipt.delivered;
            rd2.retry=receipt.retry;
            processor_.processMessage(tmIds,rd2);
        }
    }
    catch (std::exception& exc) {
        smsc_log_error(log_, "Failed to process response. Details: %s", exc.what());
    }
    catch (...) {
        smsc_log_error(log_, "Failed to process response.");
    }
}


void SmscConnector::processReceipt( const ResponseData& rd, bool internal )
{
    ReceiptId receiptId(rd.msgId);
    if (!internal) {
        smsc_log_info(log_, "Receipt(%s): smscMsgId='%s', delivered=%d, retry=%d",
                      smscId_.c_str(),
                      receiptId.getMessageId(), rd.accepted, rd.retry);
    } else {
        smsc_log_info(log_, "Response/Receipt(%s) msgId='%s' is timed out. Cleanup.",
                      smscId_.c_str(),
                      receiptId.getMessageId());
    }
    
    if (!internal) {
        MutexGuard guard(receiptsLock);
        ReceiptData* receiptPtr = receipts.GetPtr(receiptId);
        if (receiptPtr) // attach & return;
        {   
            receiptPtr->receipted = true;
            receiptPtr->delivered = rd.accepted;
            receiptPtr->retry     = rd.retry;
            return;
        }

        receipts.Insert(receiptId, ReceiptData(true, rd.accepted, rd.retry));
        MutexGuard recptGuard(receiptWaitQueueLock);
        receiptWaitQueue.Push(ReceiptTimer(time(NULL)+processor_.getReceiptWaitTime(), receiptId));
    }
    
    try
    {

      TaskMsgId tmIds;
      bool needProcess = false;
      if(jstore_->jstore.Lookup(receiptId, tmIds))
      {
        MutexGuard guard(receiptsLock);
        ReceiptData* receiptPtr = receipts.GetPtr(receiptId);
        if (receiptPtr)
        { 
          receipts.Delete(receiptId);
          needProcess = true;
        }
      }
      
      if (needProcess)
      {
      
        jstore_->jstore.Delete(receiptId);
        processor_.processMessage(tmIds,rd);
      }
      
    }
    catch (std::exception& exc)
    {
        smsc_log_error(log_, "Receipt(%s): Failed to process receipt. Details: %s",
                       smscId_.c_str(), exc.what());
    }
    catch (...)
    {
        smsc_log_error(log_, "Receipt(%s): Failed to process receipt.", smscId_.c_str());
    }
}


/*
bool SmscConnector::send( const std::string& abonent,
                          const std::string& message,
                          const TaskInfo& info, int seqNum )
{
    if (!connected_) {
        smsc_log_warn(log_, "SMSC Connector '%s' is not connected.", smscId_.c_str());
        // destroyMonitor_.notify();
        return false;
    }

    {
        MutexGuard mg(destroyMonitor_);
    
        SmppTransmitter* asyncTransmitter = session_->getAsyncTransmitter();
        if (!asyncTransmitter) {
            smsc_log_error(log_, "Smpp transmitter is undefined for SMSC Connector '%s'.", smscId_.c_str());
            return false;
        }
    
        Address oa, da;
        const char* oaStr = info.address.c_str();
        if (!oaStr || !oaStr[0]) oaStr = processor_.getAddress();
        if (!oaStr || !convertMSISDNStringToAddress(oaStr, oa)) {
            smsc_log_error(log_, "Invalid originating address '%s'", oaStr ? oaStr:"-");
            return false;
        }
    
        const char* daStr = abonent.c_str();
        if (!daStr || !convertMSISDNStringToAddress(daStr, da)) {
            smsc_log_error(log_, "Invalid destination address '%s'", daStr ? daStr:"-");
            return false;
        }
    
        time_t now = time(NULL);
      
        SMS sms;
        sms.setOriginatingAddress(oa);
        sms.setDestinationAddress(da);
        sms.setArchivationRequested(false);
        sms.setDeliveryReport(1);
        sms.setValidTime( (info.validityDate <= 0 || info.validityPeriod > 0) ?
                          now+info.validityPeriod : info.validityDate );
        
        sms.setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,
                           (info.replaceIfPresent) ? 1:0);
        sms.setEServiceType( (info.replaceIfPresent && info.svcType.length()>0) ?
                             info.svcType.c_str():processor_.getSvcType() );
    
        sms.setIntProperty(Tag::SMPP_PROTOCOL_ID, processor_.getProtocolId());
        sms.setIntProperty(Tag::SMPP_ESM_CLASS, (info.transactionMode) ? 2:0);
        sms.setIntProperty(Tag::SMPP_PRIORITY, 0);
        sms.setIntProperty(Tag::SMPP_REGISTRED_DELIVERY, 1);
    
        if(info.flash)
        {
            sms.setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,1);
        }
      
        const char* out = message.c_str();
        size_t outLen = message.length();
        char* msgBuf = 0;
        if(smsc::util::hasHighBit(out,outLen)) {
            size_t msgLen = outLen*2;
            msgBuf = new char[msgLen];
            ConvertMultibyteToUCS2(out, outLen, (short*)msgBuf, msgLen,
                                   CONV_ENCODING_CP1251);
            sms.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::UCS2);
            out = msgBuf;
            outLen = msgLen;
        } else {
            sms.setIntProperty(Tag::SMPP_DATA_CODING, DataCoding::LATIN1);
        }
      
        try {
    
            if (outLen <= MAX_ALLOWED_MESSAGE_LENGTH && !info.useDataSm) {
                sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, out, (unsigned)outLen);
                sms.setIntProperty(Tag::SMPP_SM_LENGTH, (unsigned)outLen);
            } else {
                sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, out,
                                   (unsigned)((outLen <= MAX_ALLOWED_PAYLOAD_LENGTH) ?
                                              outLen :  MAX_ALLOWED_PAYLOAD_LENGTH));
            }
        }
        catch (...) {
            smsc_log_error(log_, "Something is wrong with message body. Set/Get property failed");
            if (msgBuf) delete msgBuf; msgBuf = 0;
            return false;
        }
        if (msgBuf) delete msgBuf;
      
        try {
            if (info.useDataSm) {
                smsc_log_debug(log_, "Send DATA_SM");
                
                uint32_t validityDate = info.validityDate <= now ? 0 : static_cast<uint32_t>(info.validityDate - now);
                sms.setIntProperty(Tag::SMPP_QOS_TIME_TO_LIVE, (info.validityDate <= 0 || info.validityPeriod > 0) ?
                                                                static_cast<uint32_t>(info.validityPeriod) : validityDate);
                PduDataSm dataSm;
                dataSm.get_header().set_sequenceNumber(seqNum);
                dataSm.get_header().set_commandId(SmppCommandSet::DATA_SM);
                fillDataSmFromSms(&dataSm, &sms);
                asyncTransmitter->sendPdu(&(dataSm.get_header()));
            } else {
                smsc_log_debug(log_, "Send SUBMIT_SM");
    
                PduSubmitSm submitSm;
                submitSm.get_header().set_sequenceNumber(seqNum);
                submitSm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
                fillSmppPduFromSms(&submitSm, &sms);
                asyncTransmitter->sendPdu(&(submitSm.get_header()));
            }
            TrafficControl::incOutgoing();
            return true;
        } catch (const std::exception& ex) {
            smsc_log_warn(log_, "SmscConnector::send exception: %s", ex.what());
        } catch (...) {
            smsc_log_warn(log_, "SmscConnector::send unknown exception");
        }
    }
    reconnect();
    return false;
}
 */


void SmscConnector::clearHashes()
{
    {
        MutexGuard mg(taskIdsBySeqNumMonitor);
        taskIdsBySeqNum.Empty();
    }
    {
        MutexGuard mg(responseWaitQueueLock);
        responseWaitQueue.Empty();
    }
}

}  //infosme
}  //smsc
