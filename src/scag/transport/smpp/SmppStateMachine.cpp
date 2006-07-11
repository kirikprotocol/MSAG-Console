#include "SmppStateMachine.h"
#include "core/buffers/XHash.hpp"
#include "scag/re/RuleEngine.h"
#include "scag/sessions/SessionManager.h"
#include "scag/stat/StatisticsManager.h"
#include "system/status.h"

namespace scag{
namespace transport{
namespace smpp{

namespace buf=smsc::core::buffers;
using namespace scag::stat;

struct StateMachine::ResponseRegistry
{
  smsc::logger::Logger* log;
  int timeout;

  ResponseRegistry()
  {
    log=0;
    timeout=60;
  }
  struct RegKey{
    int uid;
    int seq;
    bool operator==(const RegKey& key) {
      return uid==key.uid && seq==key.seq;
    }
  };
  struct ListValue{
    RegKey key;
    time_t insTime;
  };

  typedef std::list<ListValue> TimeOutList;
  TimeOutList toList;

  struct RegValue{
    SmppCommand cmd;
    int dlgId;
    TimeOutList::iterator it;
  };
  struct HashFunc{
    static unsigned int CalcHash(const RegKey& key) {
      return key.uid<<9+key.seq;
    }
  };

  buf::XHash<RegKey,RegValue,HashFunc> reg;
  sync::Mutex mtx;

  void Register(int uid,int seq,SmppCommand& cmd)
  {
    sync::MutexGuard mg(mtx);
    if(!log)log=smsc::logger::Logger::getInstance("respreg");
    info2(log,"register %d/%d",uid,seq);
    RegKey key;
    key.uid=uid;
    key.seq=seq;
    RegValue val;
    ListValue lv;
    lv.key=key;
    lv.insTime=time(NULL);
    toList.push_back(lv);
    val.cmd=cmd;
    val.it=toList.end();
    val.it--;
    val.dlgId=cmd->get_dialogId();
    cmd->set_dialogId(seq);
    reg.Insert(key,val);
  }

  bool Get(int uid,int seq,SmppCommand& cmd)
  {
    sync::MutexGuard mg(mtx);
    RegKey key;
    key.uid=uid;
    key.seq=seq;
    RegValue* ptr=reg.GetPtr(key);
    info2(log,"get %d/%d - %s",uid,seq,ptr?"ok":"not found");
    if(!ptr)return 0;
    cmd=ptr->cmd;
    cmd->set_dialogId(ptr->dlgId);
    toList.erase(ptr->it);
    reg.Delete(key);
    return true;
  }

  bool getExpiredCmd(SmppCommand& cmd)
  {
    sync::MutexGuard mg(mtx);
    if(toList.empty())return false;
    time_t now=time(NULL);
    if(now-toList.front().insTime<timeout)
    {
      return false;
    }
    RegValue* ptr=reg.GetPtr(toList.front().key);
    cmd=ptr->cmd;
    cmd->set_dialogId(ptr->dlgId);
    //reg.Delete(toList.head().key);
    //toList.pop_front();
    return true;
  }

};

StateMachine::ResponseRegistry StateMachine::reg;

int StateMachine::Execute()
{
  SmppCommand cmd;
  while(!isStopping)
  {
    try{
      if(queue->getCommand(cmd))
      {
        smsc_log_debug(log,"Exec: processing command %d(%x) from %s",cmd->get_commandId(),cmd->get_commandId(),cmd.getEntity()?cmd.getEntity()->getSystemId():"");
        switch(cmd->get_commandId())
        {
          case SUBMIT:          processSubmit(cmd);         break;
          case SUBMIT_RESP:     processSubmitResp(cmd);     break;
          case DELIVERY:        processDelivery(cmd);       break;
          case DELIVERY_RESP:   processDeliveryResp(cmd);   break;
          case DATASM:          processDataSm(cmd);         break;
          case DATASM_RESP:     processDataSmResp(cmd);     break;
          case PROCESSEXPIREDRESP: processExpiredResps();   break;
          default:
            smsc_log_warn(log,"Unprocessed command id %d",cmd->get_commandId());
            break;
        }
      }
    }catch(std::exception& e)
    {
      smsc_log_error(log,"Exception in state machine:%s",e.what());
    }
  }
  return 0;
}

void StateMachine::registerEvent(int event, SmppEntity* src, SmppEntity* dst, const char* rid, int errCode)
{
    char* src_id;
    char* dst_id = NULL;
    bool srcType, dstType = false;

    smsc_log_debug(log, "SmppStateMachine Event:%d", event);

    src_id = (char*)src->info.systemId;
    srcType = src->info.type == etSmsc;

    if(dst)
    {
        dst_id = (char*)dst->info.systemId;
        dstType = dst->info.type == etSmsc;
    }

    if(rid)
        Statistics::Instance().registerEvent(SmppStatEvent(src_id, srcType, dst_id, dstType, rid, event, errCode));
    else
        Statistics::Instance().registerEvent(SmppStatEvent(src_id, srcType, dst_id, dstType, event, errCode));
}

void StateMachine::processSubmit(SmppCommand& cmd)
{
  router::RouteInfo ri;
  SMS& sms=*cmd->get_sms();
  SmsCommand& smscmd=cmd->get_smsCommand();
  smscmd.orgSrc=sms.getOriginatingAddress();
  smscmd.orgDst=sms.getDestinationAddress();
  SmppEntity *src=cmd.getEntity();
  SmppEntity *dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
  if(!dst)
  {
    smsc_log_info(log,"Submit: noroute %s(%s)->%s",
    sms.getOriginatingAddress().toString().c_str(),
    src->getSystemId(),
    sms.getDestinationAddress().toString().c_str());
    SubmitResp(cmd,smsc::system::Status::NOROUTE);
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }

  sms.setRouteId(ri.routeId);
  sms.setSourceSmeId(src->getSystemId());
  sms.setDestinationSmeId(dst->getSystemId());

  cmd->set_serviceId(ri.serviceId);

  int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
            sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
  int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

  smsc_log_debug(log, "Submit: UMR=%d, USSD_OP=%d", umr, ussd_op);

  scag::sessions::CSessionKey key;
  scag::sessions::SessionPtr session;
  key.abonentAddr=sms.getDestinationAddress();
  if (ussd_op < 0) // SMPP, No USSD specific flags
  {
    if (umr < 0) {
        key.USR = 0;
        session=scag::sessions::SessionManager::Instance().newSession(key);
        sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
    }
    else {
        key.USR = umr;
        smsc_log_debug(log, "SMPP Submit: Continue, UMR=%d", umr);
        session=scag::sessions::SessionManager::Instance().getSession(key);
        if (!session.Get()) {
            session=scag::sessions::SessionManager::Instance().newSession(key);
            sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
            smsc_log_warn(log, "SMPP Submit: Session for USR=%d not found, created new USR=%d", umr, key.USR);
        }
    }
  }
  else // USSD Dialog
  {
    if (umr < 0)
    {
        if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_REQUEST) { // New service USSD dialog
            // TODO: deny service initiated dialog !!!
            smsc_log_debug(log, "USSD Submit: Begin service dialog...");
            session=scag::sessions::SessionManager::Instance().newSession(key);
                sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
            smsc_log_debug(log, "USSD Submit: Created session USR=%d", key.USR);
        }
        else {
            smsc_log_warn(log, "USSD Submit: UMR is not specified");
            SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
            registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, -1);
            return;
        }
    }
    else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_REQUEST) { // Continue USSD dialog
        key.USR = umr; umr = dst->getUMR(key.abonentAddr, key.USR);
        smsc_log_debug(log, "USSD Submit: Continue, UMR=%d", umr);
        session=scag::sessions::SessionManager::Instance().getSession(key);
        if (!session.Get()) {
            smsc_log_warn(log, "USSD Submit: USR=%d is invalid, no session", key.USR);
            SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
            registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
            return;
        }
        sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, umr);
    }
    else if (ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_RESPONSE) { // End user USSD dialog
        key.USR = umr; umr = dst->getUMR(key.abonentAddr, key.USR);
        smsc_log_debug(log, "USSD Submit: End user dialog, UMR=%d", umr);
        session=scag::sessions::SessionManager::Instance().getSession(key);
        if (!session.Get()) {
            smsc_log_warn(log, "USSD Submit: USR=%d is invalid, no session", key.USR);
            SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
            registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
            return;
        }
        sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, umr);
    }
    else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSN_REQUEST) { // End service USSD dialog
        smsc_log_debug(log, "USSD Submit: End service dialog, UMR=%d", umr);
        if (dst) dst->delUMRMapping(key.abonentAddr, umr);
        // TODO: implement it !!!
    }
    else {
        smsc_log_warn(log, "USSD Submit: USSD_OP=%d is invalid", ussd_op);
        SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM); // TODO: status
        registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
        return;
    }
  }

  smsc_log_debug(log, "Submit: RuleEngine processing...");
  scag::re::RuleStatus st=scag::re::RuleEngine::Instance().process(cmd,*session);
  smsc_log_debug(log, "Submit: RuleEngine procesed.");

  if(!st.status)
  {
    smsc_log_info(log,"Submit: RuleEngine returned result=%d",st.result);
    SubmitResp(cmd, st.temporal ? smsc::system::Status::RX_T_APPN :
                                  smsc::system::Status::RX_P_APPN);
    scag::sessions::SessionManager::Instance().releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  try{
    int newSeq=dst->getNextSeq();
    reg.Register(dst->getUid(),newSeq,cmd);
    if(sms.hasBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS))
    {
      sms.getMessageBody().dropProperty(Tag::SMSC_UNKNOWN_OPTIONALS);
    }
    dst->putCommand(cmd);
    registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
  } catch(std::exception& e) {
    registerEvent(scag::stat::events::smpp::FAILED, src, dst, (char*)ri.routeId, -1);
    smsc_log_info(log,"Submit:Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
  }
  scag::sessions::SessionManager::Instance().releaseSession(session);
}

void StateMachine::processSubmitResp(SmppCommand& cmd)
{
  int rs = -2;
  scag::re::RuleStatus st;

  smsc_log_debug(log, "Submit resp: got");

  SmppCommand orgCmd;
  SmppEntity* src=cmd.getEntity();
  if(!reg.Get(src->getUid(),cmd->get_dialogId(),orgCmd))
  {
    smsc_log_warn(log,"Original submit for submit response not found. sid='%s',seq='%d'",src->getSystemId(),cmd->get_dialogId());
    registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
    return;
  }

  SmppEntity* dst=orgCmd.getEntity();
  SMS* sms=orgCmd->get_sms();
  cmd->get_resp()->set_sms(sms);
  cmd->set_serviceId(orgCmd->get_serviceId());
  cmd->set_operationId(orgCmd->get_operationId());

  sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
  sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);

  scag::sessions::CSessionKey key;
  key.abonentAddr=sms->getDestinationAddress();
  int umr = sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
  int ussd_op = sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;
  key.USR = (ussd_op < 0) ? umr : src->getUSR(key.abonentAddr, umr);

  scag::sessions::SessionPtr session=scag::sessions::SessionManager::Instance().getSession(key);
  if(!session.Get())
  {
    smsc_log_warn(log,"Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::RX_T_APPN);
    rs = smsc::system::Status::RX_T_APPN;
  }
  else
  {
    smsc_log_debug(log, "Submit resp: RuleEngine processing...");
    st=scag::re::RuleEngine::Instance().process(cmd,*session);
    smsc_log_debug(log, "Submit resp:RuleEngine  processed");
    if(!st.status)
        rs = st.result;
  }

  if (ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_RESPONSE ||
      ussd_op == smsc::smpp::UssdServiceOpValue::USSN_REQUEST)
      src->delUSRMapping(key.abonentAddr, key.USR); // Empty mapping on USSD dialog end

  cmd->set_dialogId(orgCmd->get_dialogId());
  if (!(cmd->get_resp()->get_messageId())) {
    smsc_log_warn(log, "Submit resp: messageId is null");
    cmd->get_resp()->set_messageId("");
  }

  try {
      dst->putCommand(cmd);
  } catch(std::exception& e) {
    smsc_log_warn(log,"SubmitResp:Failed to put command into %s:%s",dst->getSystemId(),e.what());
    rs = -1;
  }

  if(rs != -2 || !st.status)
    registerEvent(scag::stat::events::smpp::RESP_FAILED, src, dst, (char*)sms->getRouteId(), rs);
  else
    registerEvent(scag::stat::events::smpp::RESP_OK, src, dst, (char*)sms->getRouteId(), -1);

  if(session.Get())scag::sessions::SessionManager::Instance().releaseSession(session);
  cmd->get_resp()->set_sms(0);
}

void StateMachine::processDelivery(SmppCommand& cmd)
{
  router::RouteInfo ri;
  SMS& sms=*(cmd->get_sms());
  SmsCommand& smscmd=cmd->get_smsCommand();
  smscmd.orgSrc=sms.getOriginatingAddress();
  smscmd.orgDst=sms.getDestinationAddress();
  SmppEntity *src=cmd.getEntity();
  SmppEntity *dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
  if(!dst)
  {
    smsc_log_info(log,"Delivery: no route %s(%s)->%s",
      sms.getOriginatingAddress().toString().c_str(),
      src->getSystemId(),
      sms.getDestinationAddress().toString().c_str());
    DeliveryResp(cmd,smsc::system::Status::NOROUTE);
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }

  sms.setRouteId(ri.routeId);
  sms.setSourceSmeId(src->getSystemId());
  sms.setDestinationSmeId(dst->getSystemId());

  cmd->set_serviceId(ri.serviceId);

  int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
            sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
  int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

  smsc_log_debug(log, "Delivery: UMR=%d, USSD_OP=%d", umr, ussd_op);

  scag::sessions::CSessionKey key;
  scag::sessions::SessionPtr session;
  key.abonentAddr=sms.getOriginatingAddress();
  if (ussd_op < 0) // SMPP, No USSD specific flags
  {
      if (umr < 0) {
          key.USR = 0;
          session=scag::sessions::SessionManager::Instance().newSession(key);
          sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
      }
      else {
          key.USR = umr;
          smsc_log_debug(log, "SMPP Delivery: Continue, UMR=%d", umr);
          session=scag::sessions::SessionManager::Instance().getSession(key);
          if (!session.Get()) {
              session=scag::sessions::SessionManager::Instance().newSession(key);
              sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
              smsc_log_warn(log, "SMPP Delivery: Session for USR=%d not found, created new USR=%d", umr, key.USR);
          }
      }
  }
  else // USSD Dialog
  {
      if (umr < 0) {
          smsc_log_warn(log, "USSD Delivery: UMR is not specified");
          DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM);
          registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
          return;
      }
      else if (ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_INDICATION) { // New user USSD dialog
          smsc_log_debug(log, "USSD Delivery: Begin user dialog, UMR=%d", umr);
          session=scag::sessions::SessionManager::Instance().newSession(key);
          sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
          smsc_log_debug(log, "USSD Delivery: Creating mapping USR=%d, UMR=%d", key.USR, umr);
          if (src) src->setMapping(key.abonentAddr, umr, key.USR);
      }
      else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_CONFIRM) { // Continue USSD dialog
          key.USR = src->getUSR(key.abonentAddr, umr);
          smsc_log_debug(log, "USSD Delivery: Continue USR=%d", key.USR);
          session=scag::sessions::SessionManager::Instance().getSession(key);
          if (!session.Get()) {
              smsc_log_warn(log, "USSD Delivery: USR=%d is invalid, no session", key.USR);
              DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM);
              registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
              return;
          }
          sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
      }
      else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSN_CONFIRM) { // End service USSD dialog
          smsc_log_debug(log, "USSD Delivery: End service dialog, UMR=%d", umr);
          if (src) src->delUMRMapping(key.abonentAddr, umr); // TODO: check & implement
      }
      else {
          smsc_log_warn(log, "USSD Delivery: USSD_OP=%d is invalid", ussd_op);
          DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM); // TODO: status
          registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
          return;
      }
  }

  smsc_log_debug(log, "Delivery: RuleEngine processing...");
  scag::re::RuleStatus st=scag::re::RuleEngine::Instance().process(cmd,*session);
  smsc_log_debug(log, "Delivery: RuleEngine procesed.");

  if(!st.status)
  {
    smsc_log_info(log,"Delivery: RuleEngine returned result=%d",st.result);
    DeliveryResp(cmd, st.temporal? smsc::system::Status::RX_T_APPN :
                                   smsc::system::Status::RX_P_APPN);
    scag::sessions::SessionManager::Instance().releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.temporal? smsc::system::Status::RX_T_APPN : smsc::system::Status::RX_P_APPN);
    return;
  }


  try{
    int newSeq=dst->getNextSeq();
    reg.Register(dst->getUid(),newSeq,cmd);
    if(sms.hasBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS))
    {
      sms.getMessageBody().dropProperty(Tag::SMSC_UNKNOWN_OPTIONALS);
    }
    dst->putCommand(cmd);
    registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
  } catch(std::exception& e) {
    smsc_log_info(log,"Delivery:Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
    registerEvent(scag::stat::events::smpp::FAILED, src, dst, (char*)ri.routeId, -1);
  }
  scag::sessions::SessionManager::Instance().releaseSession(session);
}

void StateMachine::processDeliveryResp(SmppCommand& cmd)
{
    int rs = -2;
    scag::re::RuleStatus st;

  smsc_log_debug(log, "Delivery resp: got");

  SmppCommand orgCmd;
  SmppEntity* src=cmd.getEntity();
  if(!reg.Get(src->getUid(),cmd->get_dialogId(),orgCmd))
  {
    smsc_log_warn(log,"Original delivery for delivery response not found. sid='%s',seq='%d'",src->getSystemId(),cmd->get_dialogId());
    registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
    return;
  }

  SmppEntity* dst=orgCmd.getEntity();
  SMS* sms=orgCmd->get_sms();
  cmd->get_resp()->set_sms(sms);
  cmd->set_serviceId(orgCmd->get_serviceId());
  cmd->set_operationId(orgCmd->get_operationId());
  sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
  sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);

  scag::sessions::CSessionKey key;
  key.abonentAddr=orgCmd->get_sms()->getOriginatingAddress();
  key.USR=orgCmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);

//  if(cmd.status != CMD_OK)
//      Statistics::Instance().registerEvent(SmppStatEvent(src->info, cntRejected, -1));

  scag::sessions::SessionPtr session=scag::sessions::SessionManager::Instance().getSession(key);
  if(!session.Get())
  {
    smsc_log_warn(log,"Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::RX_T_APPN);
    rs = smsc::system::Status::RX_T_APPN;
  }
  else
  {
    smsc_log_debug(log, "Delivery resp: processing...");
    st=scag::re::RuleEngine::Instance().process(cmd,*session);
    smsc_log_debug(log, "Delivery resp: procesed.");
    if(!st.status)
        rs = st.result;
  }

  cmd->set_dialogId(orgCmd->get_dialogId());
  cmd->get_resp()->set_messageId("");

  try{
    dst->putCommand(cmd);
  } catch(std::exception& e) {
    smsc_log_warn(log,"DeliveryResp:Failed to put command into %s:%s",dst->getSystemId(),e.what());
    rs = -1;
  }

  if(rs != -2 || !st.status)
      registerEvent(scag::stat::events::smpp::RESP_FAILED, src, dst, (char*)orgCmd->get_sms()->getRouteId(), rs);
  else
      registerEvent(scag::stat::events::smpp::RESP_OK, src, dst, (char*)orgCmd->get_sms()->getRouteId(), -1);

  if(session.Get())scag::sessions::SessionManager::Instance().releaseSession(session);
  cmd->get_resp()->set_sms(0);
}

void StateMachine::SubmitResp(SmppCommand& cmd,int status)
{
  cmd.getEntity()->putCommand(
    SmppCommand::makeSubmitSmResp(
      "0",
      cmd->get_dialogId(),
      status,
      cmd->get_sms()->getIntProperty(Tag::SMPP_DATA_SM)
    )
  );
}

void StateMachine::DeliveryResp(SmppCommand& cmd,int status)
{
  cmd.getEntity()->putCommand(
    SmppCommand::makeDeliverySmResp(
      "0",
      cmd->get_dialogId(),
      status
    )
  );
}

void StateMachine::processDataSm(SmppCommand& cmd)
{
  router::RouteInfo ri;
  SMS& sms=*(cmd->get_sms());
  SmsCommand& smscmd=cmd->get_smsCommand();
  smscmd.orgSrc=sms.getOriginatingAddress();
  smscmd.orgDst=sms.getDestinationAddress();
  SmppEntity *src=cmd.getEntity();
  SmppEntity *dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
  if(!dst)
  {
    smsc_log_info(log,"DataSm: no route %s(%s)->%s",
      sms.getOriginatingAddress().toString().c_str(),
      src->getSystemId(),
      sms.getDestinationAddress().toString().c_str());
    DeliveryResp(cmd,smsc::system::Status::NOROUTE);
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }

  sms.setRouteId(ri.routeId);
  sms.setSourceSmeId(src->getSystemId());
  sms.setDestinationSmeId(dst->getSystemId());

  if(src->info.type==etService)
  {
    if(dst->info.type==etService)
    {
      smscmd.dir=dsdSrv2Srv;
    }else
    {
      smscmd.dir=dsdSrv2Sc;
    }
  }else
  {
    if(dst->info.type==etService)
    {
      smscmd.dir=dsdSc2Srv;
    }else
    {
      smscmd.dir=dsdSc2Sc;
    }
  }

  cmd->set_serviceId(ri.serviceId);

  int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
            sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
  int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

  smsc_log_debug(log, "Datasm: UMR=%d, USSD_OP=%d", umr, ussd_op);

  scag::sessions::CSessionKey key;
  scag::sessions::SessionPtr session;
  key.abonentAddr=sms.getOriginatingAddress();
  if (ussd_op < 0) // SMPP, No USSD specific flags
  {
      if (umr < 0) {
          key.USR = 0;
          session=scag::sessions::SessionManager::Instance().newSession(key);
          sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
      }
      else {
          key.USR = umr;
          smsc_log_debug(log, "SMPP DataSm: Continue, UMR=%d", umr);
          session=scag::sessions::SessionManager::Instance().getSession(key);
          if (!session.Get()) {
              session=scag::sessions::SessionManager::Instance().newSession(key);
              sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
              smsc_log_warn(log, "SMPP DataSm: Session for USR=%d not found, created new USR=%d", umr, key.USR);
          }
      }
  }
  smsc_log_debug(log, "DataSm: RuleEngine processing...");
  scag::re::RuleStatus st=scag::re::RuleEngine::Instance().process(cmd,*session);
  smsc_log_debug(log, "DataSm: RuleEngine procesed.");

  if(!st.status)
  {
    smsc_log_info(log,"DataSm: RuleEngine returned result=%d",st.result);
    DeliveryResp(cmd, st.temporal? smsc::system::Status::RX_T_APPN :
                                   smsc::system::Status::RX_P_APPN);
    scag::sessions::SessionManager::Instance().releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.temporal? smsc::system::Status::RX_T_APPN : smsc::system::Status::RX_P_APPN);
    return;
  }


  try{
    int newSeq=dst->getNextSeq();
    reg.Register(dst->getUid(),newSeq,cmd);
    if(sms.hasBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS))
    {
      sms.getMessageBody().dropProperty(Tag::SMSC_UNKNOWN_OPTIONALS);
    }
    dst->putCommand(cmd);
    registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
  } catch(std::exception& e) {
    smsc_log_info(log,"DataSm:Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
    registerEvent(scag::stat::events::smpp::FAILED, src, dst, (char*)ri.routeId, -1);
  }
  scag::sessions::SessionManager::Instance().releaseSession(session);
}
void StateMachine::processDataSmResp(SmppCommand& cmd)
{
    int rs = -2;
    scag::re::RuleStatus st;

  smsc_log_debug(log, "DataSmResp");

  SmppCommand orgCmd;
  SmppEntity* src=cmd.getEntity();
  if(!reg.Get(src->getUid(),cmd->get_dialogId(),orgCmd))
  {
    smsc_log_warn(log,"Original datasm for datasm response not found. sid='%s',seq='%d'",src->getSystemId(),cmd->get_dialogId());
    registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
    return;
  }

  SmppEntity* dst=orgCmd.getEntity();
  SMS* sms=orgCmd->get_sms();
  cmd->get_resp()->set_sms(sms);
  cmd->get_resp()->set_dir((orgCmd->get_smsCommand()).dir);

  cmd->set_serviceId(orgCmd->get_serviceId());
  cmd->set_operationId(orgCmd->get_operationId());
  sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
  sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);

  scag::sessions::CSessionKey key;
  key.abonentAddr=orgCmd->get_sms()->getOriginatingAddress();
  key.USR=orgCmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);

//  if(cmd.status != CMD_OK)
//      Statistics::Instance().registerEvent(SmppStatEvent(src->info, cntRejected, -1));

  scag::sessions::SessionPtr session=scag::sessions::SessionManager::Instance().getSession(key);
  if(!session.Get())
  {
    smsc_log_warn(log,"Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::RX_T_APPN);
    rs = smsc::system::Status::RX_T_APPN;
  }
  else
  {
    smsc_log_debug(log, "datasm resp: processing...");
    st=scag::re::RuleEngine::Instance().process(cmd,*session);
    smsc_log_debug(log, "datasm resp: procesed.");
    if(!st.status)
        rs = st.result;
  }

  cmd->set_dialogId(orgCmd->get_dialogId());
  cmd->get_resp()->set_messageId("");

  try{
    dst->putCommand(cmd);
  } catch(std::exception& e) {
    smsc_log_warn(log,"DataSmResp:Failed to put command into %s:%s",dst->getSystemId(),e.what());
    rs = -1;
  }

  if(rs != -2 || !st.status)
      registerEvent(scag::stat::events::smpp::RESP_FAILED, src, dst, (char*)orgCmd->get_sms()->getRouteId(), rs);
  else
      registerEvent(scag::stat::events::smpp::RESP_OK, src, dst, (char*)orgCmd->get_sms()->getRouteId(), -1);

  if(session.Get())scag::sessions::SessionManager::Instance().releaseSession(session);
  cmd->get_resp()->set_sms(0);
}

void StateMachine::processExpiredResps()
{
  SmppCommand cmd;
  while(reg.getExpiredCmd(cmd))
  {
    switch(cmd->get_commandId())
    {
      case DELIVERY:
      {
        processDeliveryResp(SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT));
      }break;
      case SUBMIT:
      {
        processSubmitResp(SmppCommand::makeSubmitSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT,false));
      }break;
      case DATASM:
      {
        processDataSmResp(SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT));
      }break;
    }
  }
}

}//smpp
}//transport
}//scag
