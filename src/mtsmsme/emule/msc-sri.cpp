static char const ident[] = "$Id$";
#include "mtsmsme/sccp/SccpProcessor.hpp"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "core/threads/Thread.hpp"
#include "mtsmsme/comp/SendRoutingInfo.hpp"
#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
#include "logger/Logger.h"
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <sme/SmppBase.hpp>
#include <string>
#include <vector>

using smsc::mtsmsme::processor::RequestProcessor;
using smsc::mtsmsme::processor::RequestProcessorFactory;
using smsc::mtsmsme::processor::SccpProcessor;
using smsc::mtsmsme::processor::RequestSender;
using smsc::mtsmsme::processor::Request;
using smsc::mtsmsme::processor::SubscriberRegistrator;
using smsc::mtsmsme::comp::SendRoutingInfoReq;
using smsc::mtsmsme::comp::SendRoutingInfoReqV2;
using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::TsmComletionListener;
using smsc::mtsmsme::processor::Message;
using smsc::mtsmsme::processor::util::packSCCPAddress;
using smsc::mtsmsme::processor::locationInfoRetrievalContext_v2;
using smsc::mtsmsme::processor::locationInfoRetrievalContext_v3;
using smsc::mtsmsme::processor::TrId;
using smsc::mtsmsme::processor::TSMSTAT;
using smsc::mtsmsme::processor::BeginMsg;
using smsc::mtsmsme::processor::util::packNumString2BCD91;
using smsc::mtsmsme::processor::util::dump;
using smsc::core::threads::Thread;
using smsc::sms::Address;
using smsc::logger::Logger;
using namespace smsc::util::config;
using namespace smsc::sme;
using namespace smsc::smpp;
using std::vector;
using std::string;

//static char msca[] = "791398699815"; // MSC address
//static char vlra[] = "791398699813"; //VLR address
//static char hlra[] = "791398699814"; //HLR address

//static char abnt[] = "85297023844"; //abonent MSISDN
// 3G 898520300400362096GP601G - failure, IMSI contains only Hong Kong MCC and zeroes, returned MSRN is from German numbers

//static char abnt[] = "85294570103"; //abonent MSISDN
// SmartTone 060809142018208E4H4 - working

//static char abnt[] = "96280994"; //abonent MSISDN
// New World Mobility  89852101206020123344 - didn't registered in MTS 

//static char abnt[] = "85267074004"; //abonent MSISDN
// PEOPLES - no resp from peer

//static char abnt[] = "?????"; //abonent MSISDN
// PCCW - didn't registered in MTS

//static char abnt[] = "6583962854"; //abonent MSISDN
// Starhuv - working

//static char abnt[] = "375296849152"; //abonent MSISDN
// Chahovcski v pribaltike 

//static char abnt[] = "639177949582"; //abonent MSISDN Omar Philippines
//static char abnt[] = "8613121567819"; //abonent MSISDN
//static char abnt[] = "8613810978987"; //abonent MSISDN
// Omar

//static char abnt[] = "79137247775"; //abonent MSISDN
// Kozlinsky in Europe

//static char abnt[] = "79096292608"; //abonent MSISDN
// Friend of AD in thai

//static uint8_t userid = 44;
//static uint8_t ssn = 192;
class MsrnSmeConfig : public SmeConfig
{
private:

    char *strHost, *strSid, *strPassword, *strSysType, *strOrigAddr;

public:

    MsrnSmeConfig(ConfigView* config) throw(ConfigException)
        : SmeConfig(), strHost(0), strSid(0), strPassword(0), strSysType(0), strOrigAddr(0)
    {
        // Mandatory fields
        strHost = config->getString("host", "smsc host wasn't defined !"); host = strHost;
        strSid = config->getString("sid", "msrn id wasn't defined !");   sid = strSid;

        port = config->getInt("port", "SMSC port wasn't defined !");
        timeOut = config->getInt("timeout", "Connect timeout wasn't defined !");

        // Optional fields
        try {
            strPassword = config->getString("password", "msrn password wasn't defined !");
            password = strPassword;
        } catch (ConfigException& exc) { password = ""; strPassword = 0; }
        try {
            strSysType = config->getString("systemType", "msrn system type wasn't defined !");
            systemType = strSysType;
        } catch (ConfigException& exc) { systemType = ""; strSysType = 0; }
        try {
            strOrigAddr = config->getString("origAddress", "msrn originating address wasn't defined !");
            origAddr = strOrigAddr;
        } catch (ConfigException& exc) { origAddr = ""; strOrigAddr = 0; }
    };

    virtual ~MsrnSmeConfig()
    {
        if (strHost) delete strHost;
        if (strSid) delete strSid;
        if (strPassword) delete strPassword;
        if (strSysType) delete strSysType;
        if (strOrigAddr) delete strOrigAddr;
    };
};
class RequestProcessorConfig {
  public:
    int ssn,user;
    Address msc;
    Address vlr;
    Address hlr;
  private:
    char* msc_str;
    char* vlr_str;
    char* hlr_str;
    Logger* logger;
  public:
    RequestProcessorConfig(Logger* _logger)
    {
      msc_str = NULL;
      vlr_str = NULL;
      hlr_str = NULL;
      logger = _logger;
    }
    void read(Manager& manager)
    {
      if (!manager.findSection("msrn.sccp"))
        throw ConfigException("\'sccp\' section is missed");

      ConfigView sccpConfig(manager, "msrn.sccp");

      try { user = sccpConfig.getInt("user_id");
      } catch (ConfigException& exc) {
        throw ConfigException("\'user_id\' is unknown or missing");
      }

      try { ssn = sccpConfig.getInt("user_ssn");
      } catch (ConfigException& exc) {
        throw ConfigException("\'user_ssn\' is unknown or missing");
      }

      try { msc_str = sccpConfig.getString("msc_gt");
      } catch (ConfigException& exc) {
        throw ConfigException("\'msc_gt\' is unknown or missing");
      }
      msc = Address(msc_str);

      try { vlr_str = sccpConfig.getString("vlr_gt");
      } catch (ConfigException& exc) {
        throw ConfigException("\'vlr_gt\' is unknown or missing");
      }
      vlr = Address(vlr_str);

      try { hlr_str = sccpConfig.getString("hlr_gt");
      } catch (ConfigException& exc) {
        throw ConfigException("\'hlr_gt\' is unknown or missing");
      }
      hlr = Address(hlr_str);
    }
};
class EmptyRequestSender: public RequestSender {
  virtual bool send(Request* request)
  {
    request->setSendResult(0); return true;
  }
};
class EmptySubscriberRegistrator: public SubscriberRegistrator {
  public:
    EmptySubscriberRegistrator(TCO* _tco) : SubscriberRegistrator(_tco) {}
    virtual void registerSubscriber(Address& imsi, Address& msisdn, Address& mgt, int period) {}
    virtual int  update(Address& imsi, Address& msisdn, Address& mgt) {return 1;}
    virtual bool lookup(Address& msisdn, Address& imsi, Address& msc) {return false;}
};
class SccpChan: public Thread {
  private:
    int st;
    RequestProcessorConfig& cfg;
    RequestProcessor* requestProcessor;
    TCO mtsms;
    EmptyRequestSender fakesender;
    EmptySubscriberRegistrator fakeHLR;
    Logger* logger;
  public:
    SccpChan(RequestProcessorConfig& _cfg, Logger* _logger): logger(_logger),
      cfg(_cfg), fakesender(),mtsms(1000),fakeHLR(&mtsms)
    {
      //configure(cfg.user,cfg.ssn,cfg.msc,cfg.vlr,cfg.hlr);
      ////
      RequestProcessorFactory* factory = 0;
          factory = RequestProcessorFactory::getInstance();
          if (!factory)
            throw Exception("RequestProcessorFactory is undefined");

          requestProcessor = factory->createRequestProcessor(&mtsms, &fakeHLR);
          if (!requestProcessor)
            throw Exception("RequestProcessor is undefined");
      ////
          requestProcessor->configure(cfg.user,cfg.ssn,cfg.msc,cfg.vlr,cfg.hlr);
    }
    void Stopping() {st = 0; requestProcessor->Stop(); }
    virtual int Execute()
    {
      st = 1;
      int result;
      while (st)
      {
        result = requestProcessor->Run();
        smsc_log_error(logger,"SccpListener exit with code: %d, sleep and restarting...", result);
        sleep(10);
      }
      return result;
    }
    void some(const char* abnt, TsmComletionListener* clbk)
    {
      //char abnt[] = "639177949582";
      uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
      cdlen = packSCCPAddress(cd, 1 /* E.164 */, abnt /* ABNT E.164 */, 6 /* HLR SSN */);
      cllen = packSCCPAddress(cl, 1 /* E.164 */, cfg.msc.value /* MSC  E.164 */, 8 /* MSC SSN */);
      {
        TSM* tsm = 0;
        tsm = mtsms.TC_BEGIN(locationInfoRetrievalContext_v2);
        if (tsm)
        {
          string msisdn(abnt);
          string fromwho(cfg.msc.value);
          SendRoutingInfoReqV2 req(msisdn,fromwho);
          tsm->setCompletionListener(clbk);
          tsm->TInvokeReq( 1 /* invoke_id */, 22 /* SRI */, req);
          tsm->TBeginReq(cdlen, cd, cllen, cl);
        }
      }
    }
  };
class FcukReq {
  public:
    char abnt[20];
    char srv[20];
};
class PduHandler: public SmppPduEventListener, public TsmComletionListener
{
  private:
    SccpChan& ss7;
  protected:

    Logger* logger;
    SmppSession* session;
    SmppTransmitter*    syncTransmitter;
    SmppTransmitter*    asyncTransmitter;
    FcukReq* request;

public:
    PduHandler(Logger* _logger, SccpChan& _ss7) : SmppPduEventListener(), logger(_logger),
        session(0),syncTransmitter(0), asyncTransmitter(0), ss7(_ss7),request(0)
    {

    };
    virtual ~PduHandler() {};

    void sendReply(char* msrn)
    {
      if (!request) return;
      PduSubmitSm  sm;

      sm.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
      sm.get_header().set_commandStatus(0);
      sm.get_header().set_sequenceNumber(session->getNextSeq());

      sm.get_message().set_serviceType("");
      sm.get_message().get_source().set_typeOfNumber(1),
      sm.get_message().get_source().set_numberingPlan(1);
      sm.get_message().get_source().set_value(request->abnt);

      sm.get_message().get_dest().set_typeOfNumber(0),
      sm.get_message().get_dest().set_numberingPlan(1);
      sm.get_message().get_dest().set_value(request->srv);

      sm.get_message().set_esmClass(0);
      sm.get_message().set_protocolId(0);
      sm.get_message().set_priorityFlag(0);
      sm.get_message().set_scheduleDeliveryTime("");
      sm.get_message().set_validityPeriod("");
      sm.get_message().set_registredDelivery(0);
      sm.get_message().set_replaceIfPresentFlag(0);
      sm.get_message().set_dataCoding(0);
      sm.get_message().set_smDefaultMsgId(0);
      sm.get_message().set_shortMessage(msrn,(int)strlen(msrn));

      smsc_log_info(logger, "Sending answer \'%s\'",msrn);
      asyncTransmitter->sendPdu(&(sm.get_header()));
      delete(request);
      request = 0;
    }

    virtual void complete(int status)
    {
      char dflt[] = "error";
      sendReply(dflt);
    }
    virtual void complete(Message& msg)
    {
      char dflt[] = "error";
      if (msg.isComponentPresent() && (msg.getOperationCode() == 22))
      {
        using smsc::mtsmsme::comp::SendRoutingInfoConfV2;
        std::vector<unsigned char> sriconfbuf;
        sriconfbuf = msg.getComponent();
        SendRoutingInfoConfV2 sriconf(logger);
        sriconf.decode(sriconfbuf);
        smsc_log_error(logger, "Callback called");
        sendReply(sriconf.getMSRN());
      }
      else
      {
        sendReply(dflt);
      }
    }
    void setSmppSession(SmppSession* _session) {
      session = _session;
      syncTransmitter = session->getSyncTransmitter();
      asyncTransmitter = session->getAsyncTransmitter();
    }

    void processIncomingRequest(SmppHeader *pdu_ptr)
    {
      PduDeliverySm& req = *((PduDeliverySm*) pdu_ptr);
      //char abnt[] = "639177949582";
      const char* abnt = req.get_message().get_dest().get_value();
      const char* srv = req.get_message().get_source().get_value();
      smsc_log_info(logger, "Got request for %s from %s",abnt,srv);

      PduDeliverySmResp rsp;
      rsp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      rsp.get_header().set_sequenceNumber(pdu_ptr->get_sequenceNumber());

      if (request)
      {
        rsp.get_header().set_commandStatus(SmppStatusSet::ESME_RMSGQFUL);
      }
      else
      {
        rsp.get_header().set_commandStatus(SmppStatusSet::ESME_ROK);
      }
      rsp.set_messageId("");
      smsc_log_info(logger, "Sending delivery_res");
      asyncTransmitter->sendDeliverySmResp(rsp);
      if (!request)
      {
        request = new FcukReq();
        strncpy(request->abnt,abnt,20);
        strncpy(request->srv,srv,20);
        smsc_log_info(logger, "Sending SS7 request");
        ss7.some(abnt, this);
      }
    }
    void processResponce(SmppHeader *pdu)
    {
        if (!pdu) return;

        int seqNum = pdu->get_sequenceNumber();
        int status = pdu->get_commandStatus();

        // search and report status to Request via RequestController
        smsc_log_info(logger, "receive response for seqNum=%d (status=%d)", seqNum, status);
        //controller.stateRequest(seqNum, status);
    }
    void handleTimeout(int seqNum)
    {
      //int status = DELIVERYTIMEDOUT;
      //smsc_log_info(logger, "receive timeout for seqNum=%d (status=%d)", seqNum, status);
      smsc_log_info(logger, "receive timeout for seqNum=%d", seqNum);
      //controller.stateRequest(seqNum, status);
    }
    void handleEvent(SmppHeader *pdu)
    {
//        if (bMTSMSmeIsConnecting) {
//            mtsmSmeReady.Wait(mtsmSmeReadyTimeout);
//            if (bMTSMSmeIsConnecting) {
//                disposePdu(pdu);
//                return;
//            }
//        }

        switch (pdu->get_commandId())
        {
          case SmppCommandSet::DELIVERY_SM:
            processIncomingRequest(pdu);
            //sendReply(pdu);
            break;
        case SmppCommandSet::SUBMIT_SM_RESP:
            processResponce(pdu);
            break;
        case SmppCommandSet::ENQUIRE_LINK: case SmppCommandSet::ENQUIRE_LINK_RESP:
            break;
        case SmppCommandSet::CANCEL_SM_RESP:
        default:
            smsc_log_debug(logger, "Received unsupported Pdu %d !", pdu->get_commandId());
            break;
        }

        disposePdu(pdu);
    }
    void handleError(int errorCode)
    {
        smsc_log_error(logger, "Transport error handled! Code is: %d", errorCode);
        //setNeedReconnect(true);
    }
};
//static Logger *logger = 0;
class SmppChan: public Thread {
  private:
    MsrnSmeConfig& cfg;
    PduHandler pduhdlr;
    Logger* logger;
    SmppSession session;
  public:
    SmppChan(MsrnSmeConfig& _cfg, Logger* _logger, SccpChan& _ss7): cfg(_cfg),logger(_logger),
      pduhdlr(_logger,_ss7), session(_cfg,&pduhdlr)
    {
      pduhdlr.setSmppSession(&session);
    }
    virtual int Execute()
    {
      smsc_log_info(logger, "Connecting to SMSC ... ");
      while (1)
      {
        try
        {
          session.connect();
        }
        catch (SmppConnectException& exc)
        {
          const char* msg = exc.what();
          smsc_log_error(logger, "Connect to SMSC failed. Cause: %s", (msg) ? msg:"unknown");
          sleep(cfg.timeOut);
          session.close();
          continue;
        }
        smsc_log_info(logger, "Connected.");
        while(1) sleep(1);
      }
      return 0;
    }
  };
int main(int argc, char** argv)
{
  smsc::logger::Logger::Init();
  Logger* logger = smsc::logger::Logger::getInstance("srireq");
  smsc_log_info(logger, "SRI Requester");
  try
  {

    Manager::init("config.xml");
    Manager& manager = Manager::getInstance();

    ConfigView smscConfig(manager, "msrn.smsc");
    MsrnSmeConfig cfg(&smscConfig);

    RequestProcessorConfig rp_cfg(logger);
    rp_cfg.read(manager);


    SccpChan ss7(rp_cfg,logger);

    SmppChan smpp(cfg,logger,ss7);

    ss7.Start();
    smpp.Start();

    sleep(10);
    while(true)
    {
      sleep(1);
    }
  } catch (std::exception& ex)
  {
    smsc_log_error(logger, " cought unexpected exception [%s]", ex.what());
  }
  return 0;
}
