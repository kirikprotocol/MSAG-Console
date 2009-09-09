#include <util/smstext.h>
#include "SmscConnector.h"
#include "TrafficControl.hpp"

namespace {

std::string cgetString( smsc::util::config::ConfigView& cv, const char* tag, const char* what )
{
    std::auto_ptr<char> str(cv.getString(tag,what));
    return std::string(str.get());
}

}

namespace smsc {
namespace infosme {

using smsc::core::synchronization::MutexGuard;


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
    } catch (ConfigException&) {}
    try {
        rv.systemType = ::cgetString(config,"systemType","InfoSme system type wasn't defined !");
    } catch (ConfigException&) {}
    try {
        rv.origAddr = ::cgetString(config,"origAddress",
                                   "InfoSme originating address wasn't defined !");
    } catch (ConfigException&) {}
    return rv;
}


SmscConnector::SmscConnector(TaskProcessor& processor, const smsc::sme::SmeConfig& cfg, const string& smscId):
processor_(processor), 
logger_(Logger::getInstance("smsc.infosme.connector")),
listener_(processor_, smscId, logger_),
session_(cfg, &listener_),
smscId_(smscId),
timeout_(cfg.timeOut),
stopped_(false),
connected_(false)
{
  listener_.setSyncTransmitter(session_.getSyncTransmitter());
  listener_.setAsyncTransmitter(session_.getAsyncTransmitter());
}

SmscConnector::~SmscConnector()
{
    stop();
    WaitFor();
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
    smsc_log_warn(logger_, "FIXME: updateConfig on '%s'... ", smscId_.c_str());
}

bool SmscConnector::isStopped() const {
  return stopped_;
}

int SmscConnector::Execute() { 
  while (!isStopped())
  {
    // after call to isNeedStop() was completed all signals is locked.
    // any thread being started from this point has signal mask with all signals locked 
      smsc_log_info(logger_, "Connecting to SMSC id='%s'... ", smscId_.c_str());
      try
      {
          session_.connect();
          MutexGuard mg(connectMonitor_);
          connected_ = true;
      }
      catch (SmppConnectException& exc)
      {
          const char* msg = exc.what();
          smsc_log_error(logger_, "Connect to SMSC id='%s' failed. Cause: %s", smscId_.c_str(), (msg) ? msg:"unknown");
          //bInfoSmeIsConnecting = false;
          if (exc.getReason() == SmppConnectException::Reason::bindFailed) throw;
          sleep(timeout_);
          session_.close();
          MutexGuard mg(connectMonitor_);
          connected_ = false;
          continue;
      }
      smsc_log_info(logger_, "Connected to SMSC id='%s'.", smscId_.c_str());
      {
        MutexGuard mg(connectMonitor_);
        while (connected_ && !isStopped()) {
          connectMonitor_.wait();
        }
        if (!connected_) {
          smsc_log_info(logger_, "Need Reconnect to SMSC id='%s'.", smscId_.c_str());
        }
      }
      session_.close();
  }
  smsc_log_info(logger_, "SMSC Connector '%s' stopped", smscId_.c_str());
  return 1; 
}

int SmscConnector::getSeqNum() {
  MutexGuard guard(sendLock_);
  return session_.getNextSeq();
}

bool SmscConnector::send(std::string abonent, std::string message, TaskInfo info, int seqNum) {
  {
    MutexGuard mg(connectMonitor_);
    if (!connected_) {
      smsc_log_warn(logger_, "SMSC Connector '%s' is not connected.", smscId_.c_str());
      connectMonitor_.notify();
      return false;
    }
  }
 {
    MutexGuard guard(sendLock_);
    SmppTransmitter* asyncTransmitter = session_.getAsyncTransmitter();
    if (!asyncTransmitter) {
        smsc_log_error(logger_, "Smpp transmitter is undefined for SMSC Connector '%s'.", smscId_.c_str());
        return false;
    }
  
    Address oa, da;
    const char* oaStr = info.address.c_str();
    if (!oaStr || !oaStr[0]) oaStr = processor_.getAddress();
    if (!oaStr || !convertMSISDNStringToAddress(oaStr, oa)) {
        smsc_log_error(logger_, "Invalid originating address '%s'", oaStr ? oaStr:"-");
        return false;
    }
    const char* daStr = abonent.c_str();
    if (!daStr || !convertMSISDNStringToAddress(daStr, da)) {
        smsc_log_error(logger_, "Invalid destination address '%s'", daStr ? daStr:"-");
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
        smsc_log_error(logger_, "Something is wrong with message body. Set/Get property failed");
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
      smsc_log_warn(logger_, "SmscConnector::send exception: %s", ex.what());
    } catch (...) {
      smsc_log_warn(logger_, "SmscConnector::send unknown exception");
    }
 }
  reconnect();
  return false;
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
    sbm.get_optional().set_messagePayload(txt.c_str(),txt.length());
  }
  sbm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
  sbm.get_header().set_sequenceNumber(getSeqNum());
  PduSubmitSmResp* resp=session_.getSyncTransmitter()->submit(sbm);
  if(!resp)
  {
    return SmppStatusSet::ESME_RUNKNOWNERR;
  }
  uint32_t rv=resp->get_header().get_commandStatus();
  disposePdu((SmppHeader*)resp);
  return rv;
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

}  //infosme
}  //smsc
