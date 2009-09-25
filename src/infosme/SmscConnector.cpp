#include "util/smstext.h"
#include "SmscConnector.h"
#include "TrafficControl.hpp"
#include "TaskProcessor.h"
#include "core/buffers/JStore.hpp"
#include "util/config/region/RegionFinder.hpp"

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
    MessageGuard(Task* argTask, const Message& theMsg) :
    task(argTask), msg(theMsg), state(msgFailed)
    {
    }

    ~MessageGuard()
    {
        if (state == msgFailed ) {
            TaskProcessor::retryMessage(task,msg.id);
        } else if ( state == msgSuspended ) {
            task->putToSuspendedMessagesQueue(msg);
        }
    }

    void processed() {
        state = msgProcessed;
    }
    void suspended() {
        state = msgSuspended;
    }

private:
    Task* task;
    const Message& msg;
    MessageState state;
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
        MutexGuard mg(processor.connectMonitor_);
        if ( --processor.usage_ == 0 ) processor.connectMonitor_.notifyAll();
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
        const std::string jLocation = storeLocation + "/jstore";
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


// NOTE: this class is not thread-safe
class SmscConnector::RegionTrafficControl
{
public:
    RegionTrafficControl( smsc::logger::Logger* logger ) : log_(logger) {}
    ~RegionTrafficControl();
    bool speedLimitReached( Task* task, const Message& message );
private:
    smsc::logger::Logger* log_;
    typedef std::map<std::string, TimeSlotCounter<int>* > timeSlotsHashByRegion_t;
    timeSlotsHashByRegion_t timeSlotsHashByRegion_;
};


SmscConnector::RegionTrafficControl::~RegionTrafficControl()
{
    for ( timeSlotsHashByRegion_t::iterator i = timeSlotsHashByRegion_.begin();
          i != timeSlotsHashByRegion_.end();
          ++i ) {
        delete i->second;
    }
}


bool SmscConnector::RegionTrafficControl::speedLimitReached( Task* task, const Message& message )
{
    uint32_t taskId = task->getId();
    smsc_log_debug( log_, "TaskProcessor::controlTrafficSpeedByRegion::: TaskId=[%d]: check region(regionId=%s) bandwidth limit exceeding", taskId, message.regionId.c_str());
    const smsc::util::config::region::Region* region = smsc::util::config::region::RegionFinder::getInstance().getRegionById(message.regionId);
    if ( ! region ) return true;

    timeSlotsHashByRegion_t::iterator iter = timeSlotsHashByRegion_.lower_bound(message.regionId);
    if ( iter == timeSlotsHashByRegion_.end() || iter->first != message.regionId ) {
        // not found
        smsc_log_debug(log_, "TaskProcessor::controlTrafficSpeedByRegion::: TaskId=[%d]: insert timeSlot to hash for regionId=%s", taskId, message.regionId.c_str());
        iter = timeSlotsHashByRegion_.insert(iter,std::make_pair(message.regionId, new TimeSlotCounter<int>(1,1)));
    }
    TimeSlotCounter<int>* outgoing = iter->second;
    int out = outgoing->Get();

    bool regionTrafficLimitReached = (out >= region->getBandwidth());
    smsc_log_debug(log_, "TaskProcessor::controlTrafficSpeedByRegion::: TaskId=[%d]: regionTrafficLimitReached=%d, region bandwidth=%d, current sent messages during one second=%d",
                   taskId, regionTrafficLimitReached, region->getBandwidth(), out);
    // check max messages per sec. limit. if limit was reached then put message to queue of suspended messages.
    if ( regionTrafficLimitReached ) { return true; }

    outgoing->Inc();
    return false;
}

// ===================================================================

smsc::sme::SmeConfig SmscConnector::readSmeConfig( ConfigView& config )
{
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
        rv.systemType = ::cgetString(config,"systemType","InfoSme system type wasn't defined !");
    } catch (smsc::util::config::ConfigException&) {}
    try {
        rv.origAddr = ::cgetString(config,"origAddress",
                                   "InfoSme originating address wasn't defined !");
    } catch (smsc::util::config::ConfigException&) {}
    return rv;
}


SmscConnector::SmscConnector( TaskProcessor& processor,
                              const smsc::sme::SmeConfig& cfg,
                              const string& smscId):
smscId_(smscId),
log_(Logger::getInstance("smsc.infosme.connector")),
processor_(processor),
listener_(*this, log_),
session_(new SmppSession(cfg, &listener_)),
timeout_(cfg.timeOut),
stopped_(false),
connected_(false),
usage_(0),
jstore_(0),
trafficControl_(0)
{
    listener_.setSyncTransmitter(session_->getSyncTransmitter());
    listener_.setAsyncTransmitter(session_->getAsyncTransmitter());
    jstore_ = new JStoreWrapper(processor_.getStoreLocation(),
                                smscId_,
                                processor_.getMappingRollTime(),
                                processor_.getMappingMaxChanges() );
    trafficControl_ = new RegionTrafficControl(log_);
}


SmscConnector::~SmscConnector()
{
    stop();
    WaitFor();
    // waiting until all dependent objects finished
    while ( true ) {
        MutexGuard mg(connectMonitor_);
        if ( usage_ <= 0 ) break;
        connectMonitor_.wait(100);
    }
    if ( jstore_ ) { delete jstore_; }
    if ( trafficControl_ ) { delete trafficControl_; }
}

void SmscConnector::stop() {
  MutexGuard mg(connectMonitor_);
  stopped_ = true;
  connectMonitor_.notify();
}

void SmscConnector::reconnect() {
  MutexGuard mg(connectMonitor_);
  connected_ = false;
  connectMonitor_.notify();
}

void SmscConnector::updateConfig( const smsc::sme::SmeConfig& config )
{
    smsc_log_warn(log_, "updateConfig on '%s'... ", smscId_.c_str());
    MutexGuard mg(connectMonitor_);
    session_->close();
    std::auto_ptr<SmppSession> newsess(new SmppSession(config,&listener_));
    listener_.setSyncTransmitter(newsess->getSyncTransmitter());
    listener_.setAsyncTransmitter(newsess->getAsyncTransmitter());
    std::auto_ptr<SmppSession> oldsess(session_.release());
    session_.reset(newsess.release());
    connected_ = false;
    connectMonitor_.notify();
}

bool SmscConnector::isStopped() const {
  return stopped_;
}

int SmscConnector::Execute() { 
  while (!isStopped())
  {
    // after call to isNeedStop() was completed all signals is locked.
    // any thread being started from this point has signal mask with all signals locked 
      clearHashes();
      smsc_log_info(log_, "Connecting to SMSC id='%s'... ", smscId_.c_str());
      MutexGuard mg(connectMonitor_);
      try
      {
          session_->connect();
          connected_ = true;
      }
      catch (SmppConnectException& exc)
      {
          const char* msg = exc.what();
          smsc_log_error(log_, "Connect to SMSC id='%s' failed. Cause: %s", smscId_.c_str(), (msg) ? msg:"unknown");
          //bInfoSmeIsConnecting = false;
          if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw;
          sleep(timeout_);
          session_->close();
          connected_ = false;
          continue;
      }
      smsc_log_info(log_, "Connected to SMSC id='%s'.", smscId_.c_str());
      while (connected_ && !isStopped()) {
        connectMonitor_.wait();
      }
      if (!connected_) {
        smsc_log_info(log_, "Need Reconnect to SMSC id='%s'.", smscId_.c_str());
      }
      session_->close();
  }
    clearHashes();
    smsc_log_info(log_, "SMSC Connector '%s' stopped", smscId_.c_str());
  return 1; 
}

int SmscConnector::getSeqNum() {
  MutexGuard guard(connectMonitor_);
  return session_->getNextSeq();
}


uint32_t SmscConnector::sendSms(const std::string& org,const std::string& dst,const std::string& txt,bool flash) {
  PduSubmitSm sbm;
  PduPartSm& msg=sbm.get_message();
  msg.set_source(smsc::smpp::Address2PduAddress(org.c_str()));
  msg.set_dest(smsc::smpp::Address2PduAddress(dst.c_str()));
  msg.set_esmClass(0x02);//forward mode
  if(flash)
  {
    sbm.get_optional().set_destAddrSubunit(1);
  }

  if(hasHighBit(txt.c_str(),txt.length()))
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
        MutexGuard mg(connectMonitor_);
        if ( stopped_ ) return false;
        er = new EventRunner(processReceiptMethod,*this,rd);
    }
    return processor_.invokeProcessEvent(er);
}


bool SmscConnector::invokeProcessResponse( const ResponseData& rd )
{
    EventRunner* er = 0;
    {
        MutexGuard mg(connectMonitor_);
        if ( stopped_ ) return false;
        er = new EventRunner(processResponseMethod,*this,rd);
    }
    return processor_.invokeProcessEvent(er);
}


bool SmscConnector::send( Task* task, Message& message )
{
    MessageGuard msguard(task,message);
    const TaskInfo& info = task->getInfo();

    if ( ! connected_ ) {
        msguard.suspended();
        smsc_log_debug(log_, "TaskId=[%d/%s]: SMSC id='%s' is not connected",
                       info.uid, info.name.c_str(), smscId_.c_str());
        return false;
    }

    if ( trafficControl_->speedLimitReached(task,message) ) {
        msguard.suspended();
        if ( log_->isInfoEnabled() ) {
            const smsc::util::config::region::Region* region = smsc::util::config::region::RegionFinder::getInstance().getRegionById(message.regionId);
            smsc_log_info(log_, "TaskId=[%d/%s]: Traffic for region %s with id %s was suspended",
                          info.uid, info.name.c_str(), region->getName().c_str(), region->getId().c_str());
        }
        return false;
    }

    int seqNum = getSeqNum();
    smsc_log_debug(log_, "TaskId=[%d/%s]: Sending message #%llx, seqNum=%d SMSC id='%s' region id='%s' for '%s': %s",
                   info.uid,info.name.c_str(), message.id, seqNum, smscId_.c_str(), message.regionId.c_str(),
                   message.abonent.c_str(), message.message.c_str());

    {
        MutexGuard snGuard(taskIdsBySeqNumMonitor);
        int seqNumsCount;
        if (processor_.bNeedExit || stopped_ ) return false;
        seqNumsCount = taskIdsBySeqNum.Count();
        if ( seqNumsCount > processor_.getUnrespondedMessagesMax() ) {
            msguard.suspended(); // to prevent retry
            smsc_log_debug(log_,"TaskId=[%d/%s]: too many messages queued for SMSC id='%s'",
                           info.uid, info.name.c_str(), smscId_.c_str() );
            return false;
        }
        if (taskIdsBySeqNum.Exist(seqNum))
        {
            smsc_log_warn(log_, "Sequence id=%d SMSC id='%s' was already used !", seqNum, smscId_.c_str());
            taskIdsBySeqNum.Delete(seqNum);
        }
        taskIdsBySeqNum.Insert(seqNum, TaskMsgId(info.uid, message.id));
    }
    {
        MutexGuard respGuard(responseWaitQueueLock);
        responseWaitQueue.Push(ResponseTimer(time(NULL)+processor_.getResponseWaitTime(), seqNum));
    }
    if ( ! send(message.abonent,message.message,info,seqNum) ) {

        smsc_log_error( log_, "Failed to send message #%llx for '%s'",
                        message.id, message.abonent.c_str());
        msguard.suspended();
        MutexGuard snGuard(taskIdsBySeqNumMonitor);
        if ( taskIdsBySeqNum.Delete(seqNum) ) {
            taskIdsBySeqNumMonitor.notifyAll();
        }
        return false;

    }

    // message is sent
    msguard.processed();
    smsc_log_info(log_, "TaskId=[%d/%s]: Sent message #%llx sq=%d for '%s' to SMSC '%s'",
                  info.uid, info.name.c_str(), message.id, seqNum, message.abonent.c_str(), smscId_.c_str());
    return true;
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


bool SmscConnector::convertMSISDNStringToAddress(const char* string, Address& address)
{
    try {
        Address converted(string);
        address = converted;
    } catch (...) {
        return false;
    }
    return true;
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


bool SmscConnector::send( const std::string& abonent,
                          const std::string& message,
                          const TaskInfo& info, int seqNum )
{
    {
      MutexGuard mg(connectMonitor_);
      if (!connected_) {
          smsc_log_warn(log_, "SMSC Connector '%s' is not connected.", smscId_.c_str());
          connectMonitor_.notify();
          return false;
      }
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
  
        SMS sms;
        sms.setOriginatingAddress(oa);
        sms.setDestinationAddress(da);
        sms.setArchivationRequested(false);
        sms.setDeliveryReport(1);
        sms.setValidTime( (info.validityDate <= 0 || info.validityPeriod > 0) ?
                          time(NULL)+info.validityPeriod : info.validityDate );
  
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
  
        try
        {
            if (outLen <= MAX_ALLOWED_MESSAGE_LENGTH) {
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
            PduSubmitSm sm;
            sm.get_header().set_sequenceNumber(seqNum);
            sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
            fillSmppPduFromSms(&sm, &sms);
            asyncTransmitter->sendPdu(&(sm.get_header()));
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
