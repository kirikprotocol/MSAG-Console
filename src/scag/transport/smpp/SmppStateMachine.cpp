#include "SmppStateMachine.h"
#include "core/buffers/XHash.hpp"
#include "scag/re/RuleEngine.h"
#include "scag/sessions/SessionManager.h"
#include "scag/stat/StatisticsManager.h"
#include "system/status.h"
#include "SmppUtil.h"

namespace scag{
namespace transport{
namespace smpp{

namespace buf=smsc::core::buffers;
using namespace scag::stat;

std::vector<int> StateMachine::allowedUnknownOptionals;

const int MAX_REDIRECT_CNT=10;

struct StateMachine::ResponseRegistry
{
  smsc::logger::Logger* log;
  int timeout;

  ResponseRegistry() : log(0), timeout(60) {}

  struct RegKey {
    int uid, seq;
    RegKey(int _uid=-1, int _seq=-1)
  : uid(_uid), seq(_seq) {}
    bool operator==(const RegKey& key) {
      return uid==key.uid && seq==key.seq;
    }
  };
  struct ListValue {
    RegKey key;
    time_t insTime;
  };

  typedef std::list<ListValue> TimeOutList;
  TimeOutList toList;

  struct RegValue {
    SmppCommand cmd;
    int dlgId;
    TimeOutList::iterator it;
  };
  struct HashFunc{
    static unsigned int CalcHash(const RegKey& key) {
      return key.uid<<9+key.seq;
    }
  };

  buf::XHash<RegKey, RegValue, HashFunc> reg;
  sync::Mutex mtx;

  bool Register(int uid, int seq, SmppCommand& cmd)
  {
    sync::MutexGuard mg(mtx);
    RegKey key(uid, seq);
    if (!log) log=smsc::logger::Logger::getInstance("respreg");
    if (reg.Exists(key)) { // key already registered
  smsc_log_warn(log, "register %d/%d failed", uid, seq);
  return false;
    }
    smsc_log_debug(log, "register %d/%d", uid, seq);
    RegValue val;
    ListValue lv;
    lv.key = key;
    lv.insTime = time(NULL);
    toList.push_back(lv);
    val.cmd = cmd;
    val.it = toList.end(); val.it--;
    val.dlgId = cmd->get_dialogId();
    cmd->set_dialogId(seq);
    reg.Insert(key,val);
    return true;
  }

  bool Get(int uid, int seq, SmppCommand& cmd)
  {
    sync::MutexGuard mg(mtx);
    RegKey key(uid, seq);
    RegValue* ptr=reg.GetPtr(key);
    if (!log) log=smsc::logger::Logger::getInstance("respreg");
    smsc_log_debug(log, "get %d/%d - %s", uid, seq, (ptr) ? "ok":"not found");
    if (!ptr) { // TODO: toList cleanup (find listValue by key & delete it)?
  return false;
    }
    cmd = ptr->cmd;
    cmd->set_dialogId(ptr->dlgId);
    toList.erase(ptr->it);
    reg.Delete(key);
    return true;
  }

  bool getExpiredCmd(SmppCommand& cmd)
  {
    sync::MutexGuard mg(mtx);
    if (toList.empty()) return false;
    time_t now = time(NULL);
    if ((now - toList.front().insTime) < timeout) return false;
    RegKey key = toList.front().key;
    RegValue* ptr = reg.GetPtr(key);
    if (!ptr) { // TODO: toList cleanup (find listValue by key & delete it)?
  // toList.erase(toList.begin());
  return false;
    }
    cmd = ptr->cmd;
    cmd->set_dialogId(key.seq);
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
        if (cmd->get_commandId() != 25) {
          smsc_log_debug(log,"Exec: processing command %d(%x) from %s",cmd->get_commandId(),cmd->get_commandId(),cmd.getEntity()?cmd.getEntity()->getSystemId():"");
  }
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
    uint32_t rcnt = 0;
    SmppEntity *src = NULL;
    SmppEntity *dst = NULL;
    scag::sessions::SessionPtr session;
    scag::sessions::CSessionKey key;  
    router::RouteInfo ri;
    buf::FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;
    scag::re::RuleStatus st;
    SMS& sms=*cmd->get_sms();
    SmsCommand& smscmd=cmd->get_smsCommand();

    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src=cmd.getEntity();    
    
    do{  
      dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
      if(!dst || routeId == ri.routeId)
      {
        smsc_log_info(log,"Submit: %s %s(%s)->%s", !dst ? "no route" : "redirection to the same route",
        sms.getOriginatingAddress().toString().c_str(),
        src->getSystemId(),
        sms.getDestinationAddress().toString().c_str());
        SubmitResp(cmd,smsc::system::Status::NOROUTE);
        if(session.Get())
            scag::sessions::SessionManager::Instance().releaseSession(session);                  
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        return;
      }
      routeId = ri.routeId;

      sms.setRouteId(ri.routeId);
      sms.setSourceSmeId(src->getSystemId());
      sms.setDestinationSmeId(dst->getSystemId());

      cmd->set_serviceId(ri.serviceId);

      int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
                sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
      int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                    sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

      smsc_log_debug(log, "Submit:%s UMR=%d, USSD_OP=%d. %s(%s)->%s", rcnt ? "(redirected)" : "", umr, ussd_op,
        sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str());

      key.abonentAddr=sms.getDestinationAddress();
      if (ussd_op < 0) // SMPP, No USSD specific flags
      {
        if(!session.Get())
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
        else
            session->setRedirectFlag();
      }
      else // USSD Dialog
      {
        if(session.Get())
        {
            smsc_log_warn(log, "USSD Submit: Rerouting for USSD dialog not allowed");        
            SubmitResp(cmd,smsc::system::Status::NOROUTE);
            scag::sessions::SessionManager::Instance().releaseSession(session);                  
            registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
            return;
        }
        
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
      st=scag::re::RuleEngine::Instance().process(cmd,*session);
      smsc_log_debug(log, "Submit: RuleEngine procesed.");
  
  }while(st.status == scag::re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

  if(rcnt >= MAX_REDIRECT_CNT)
  {
    smsc_log_info(log,"Submit: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
        src->getSystemId(), sms.getDestinationAddress().toString().c_str());
    SubmitResp(cmd,smsc::system::Status::NOROUTE);
    scag::sessions::SessionManager::Instance().releaseSession(session);                  
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }
  
  if(st.status != scag::re::STATUS_OK)
  {
    smsc_log_info(log,"Submit: RuleEngine returned result=%d",st.result);
    if(!st.result)
    {
        smsc_log_warn(log, "Submit: Rule failed and no error(zero rezult) returned");        
        st.result = smsc::system::Status::SYSERR;
    }
    SubmitResp(cmd, st.result);        
    scag::sessions::SessionManager::Instance().releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  try{
    int newSeq=dst->getNextSeq();
    if (!reg.Register(dst->getUid(), newSeq, cmd))
    {
      throw Exception("Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
    }
    stripUnknownSmppOptionals(sms,allowedUnknownOptionals);
    dst->putCommand(cmd);
    registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
  } catch(std::exception& e) {
    registerEvent(scag::stat::events::smpp::FAILED, src, dst, (char*)ri.routeId, -1);
    smsc_log_info(log,"Submit: Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
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
    cmd->get_resp()->set_status(smsc::system::Status::TRANSACTIONTIMEDOUT);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
    smsc_log_debug(log, "Submit resp: RuleEngine processing...");
    st=scag::re::RuleEngine::Instance().process(cmd,*session);
    smsc_log_debug(log, "Submit resp:RuleEngine  processed");
    if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "Submit resp: Rule failed and no error(zero rezult) returned");        
            st.result = smsc::system::Status::SYSERR;
        }
       
        rs = st.result;
        cmd->get_resp()->set_status(st.result);
    }
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

  if(rs != -2 || st.status != scag::re::STATUS_OK)
    registerEvent(scag::stat::events::smpp::RESP_FAILED, src, dst, (char*)sms->getRouteId(), rs);
  else
    registerEvent(scag::stat::events::smpp::RESP_OK, src, dst, (char*)sms->getRouteId(), -1);

  if(session.Get())scag::sessions::SessionManager::Instance().releaseSession(session);
  cmd->get_resp()->set_sms(0);
}

void StateMachine::processDelivery(SmppCommand& cmd)
{
    uint32_t rcnt = 0;
  SmppEntity *src = NULL;
  SmppEntity *dst = NULL;
  scag::sessions::SessionPtr session;
  scag::sessions::CSessionKey key;  
  router::RouteInfo ri;
  buf::FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;  
    scag::re::RuleStatus st;  
    SMS& sms=*(cmd->get_sms());
    SmsCommand& smscmd=cmd->get_smsCommand();
    
    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src=cmd.getEntity();
    
    do{
      dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
      if(!dst || routeId == ri.routeId)
      {
        smsc_log_info(log,"Delivery: %s %s(%s)->%s", !dst ? "no route" : "redirection to the same route",
          sms.getOriginatingAddress().toString().c_str(),
          src->getSystemId(),
          sms.getDestinationAddress().toString().c_str());
        DeliveryResp(cmd,smsc::system::Status::NOROUTE);
        if(session.Get())
            scag::sessions::SessionManager::Instance().releaseSession(session);                  
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        return;
      }
      routeId = ri.routeId;

      sms.setRouteId(ri.routeId);
      sms.setSourceSmeId(src->getSystemId());
      sms.setDestinationSmeId(dst->getSystemId());

      cmd->set_serviceId(ri.serviceId);

      int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
                sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
      int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                    sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

      smsc_log_debug(log, "Delivery:%s UMR=%d, USSD_OP=%d. %s(%s)->%s", rcnt ? "(redirected)" : "", umr, ussd_op,
        sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str());
        
      key.abonentAddr=sms.getOriginatingAddress();
      if (ussd_op < 0) // SMPP, No USSD specific flags
      {
          if(!session.Get())
          {
              key.USR = 0; // Always create new session
              session=scag::sessions::SessionManager::Instance().newSession(key);
              sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
              if (umr > 0) {
                  smsc_log_warn(log, "SMPP Delivery, UMR=%d is set. "
                                     "Created new session instead. USR=%d", umr, key.USR);
                umr = -1;
              }
          }
          else
            session->setRedirectFlag();
      }
      else // USSD Dialog
      {
            if(session.Get())
            {
                smsc_log_warn(log, "USSD Delivery: Rerouting for USSD dialog not allowed");        
                DeliveryResp(cmd,smsc::system::Status::NOROUTE);
                scag::sessions::SessionManager::Instance().releaseSession(session);                  
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, NULL, smsc::system::Status::NOROUTE);
                return;
            }
        
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
      st=scag::re::RuleEngine::Instance().process(cmd,*session);
      smsc_log_debug(log, "Delivery: RuleEngine procesed.");

  }while(st.status == scag::re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

  if(rcnt >= MAX_REDIRECT_CNT)
  {
    smsc_log_info(log,"Delivery: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s",  sms.getOriginatingAddress().toString().c_str(),
          src->getSystemId(), sms.getDestinationAddress().toString().c_str());
    
    DeliveryResp(cmd,smsc::system::Status::NOROUTE);
    scag::sessions::SessionManager::Instance().releaseSession(session);            
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }
    
  if(st.status != scag::re::STATUS_OK)
  {
    smsc_log_info(log,"Delivery: RuleEngine returned result=%d",st.result);
    if(!st.result)
    {
        smsc_log_warn(log, "Delivery: Rule failed and no error(zero rezult) returned");        
        st.result = smsc::system::Status::SYSERR;
    }
    DeliveryResp(cmd, st.result);        
    scag::sessions::SessionManager::Instance().releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  try{
    int newSeq=dst->getNextSeq();
    if (!reg.Register(dst->getUid(),newSeq,cmd))
    {
      throw Exception("Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
    }
    stripUnknownSmppOptionals(sms,allowedUnknownOptionals);
    dst->putCommand(cmd);
    registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
  } catch(std::exception& e) {
    smsc_log_info(log,"Delivery: Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
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
    cmd->get_resp()->set_status(smsc::system::Status::TRANSACTIONTIMEDOUT);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
    smsc_log_debug(log, "Delivery resp: processing...");
    st=scag::re::RuleEngine::Instance().process(cmd,*session);
    smsc_log_debug(log, "Delivery resp: procesed.");
    if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "Delivery resp: Rule failed and no error(zero rezult) returned");        
            st.result = smsc::system::Status::SYSERR;
        }
       
        rs = st.result;
        cmd->get_resp()->set_status(st.result);
    }
  }

  cmd->set_dialogId(orgCmd->get_dialogId());
  cmd->get_resp()->set_messageId("");

  try{
    dst->putCommand(cmd);
  } catch(std::exception& e) {
    smsc_log_warn(log,"DeliveryResp:Failed to put command into %s:%s",dst->getSystemId(),e.what());
    rs = -1;
  }

  if(rs != -2 || st.status != scag::re::STATUS_OK)
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

void StateMachine::DataResp(SmppCommand& cmd,int status)
{
  cmd.getEntity()->putCommand(
    SmppCommand::makeDataSmResp(
      "0",
      cmd->get_dialogId(),
      status
    )
  );
}

void StateMachine::processDataSm(SmppCommand& cmd)
{
    uint32_t rcnt = 0;
  SmppEntity *src = NULL;
  SmppEntity *dst = NULL;
  scag::sessions::SessionPtr session;
  scag::sessions::CSessionKey key;  
  router::RouteInfo ri;
  buf::FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;    
    scag::re::RuleStatus st;
    SMS& sms=*(cmd->get_sms());
    SmsCommand& smscmd=cmd->get_smsCommand();

    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src=cmd.getEntity();
    
  do{
      dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
      if(!dst || routeId == ri.routeId)
      {
        smsc_log_info(log,"DataSm: %s %s(%s)->%s", !dst ? "no route" : "redirection to the same route",        
          sms.getOriginatingAddress().toString().c_str(),
          src->getSystemId(),
          sms.getDestinationAddress().toString().c_str());
        DataResp(cmd,smsc::system::Status::NOROUTE);
        if(session.Get())
            scag::sessions::SessionManager::Instance().releaseSession(session);                  
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        return;
      }
      routeId = ri.routeId;

      sms.setRouteId(ri.routeId);
      sms.setSourceSmeId(src->getSystemId());
      sms.setDestinationSmeId(dst->getSystemId());

      if(src->info.type==etService) {
        smscmd.dir = (dst->info.type==etService) ? dsdSrv2Srv : dsdSrv2Sc;
      }
      else {
        smscmd.dir = (dst->info.type==etService) ? dsdSc2Srv : dsdSc2Sc;
      }

      cmd->set_serviceId(ri.serviceId);

      int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
                sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
      int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                    sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

      smsc_log_debug(log, "Datasm:%s UMR=%d, USSD_OP=%d. %s(%s)->%s", rcnt ? "(redirected)" : "", umr, ussd_op,
        sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str());      

      key.abonentAddr = (src->info.type == etService) ?
             sms.getDestinationAddress() : sms.getOriginatingAddress();
      if (ussd_op < 0) // SMPP, No USSD specific flags
      {
        if(!session.Get())
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
        else
            session->setRedirectFlag();
      }
      smsc_log_debug(log, "DataSm: RuleEngine processing...");
      st=scag::re::RuleEngine::Instance().process(cmd,*session);
      smsc_log_debug(log, "DataSm: RuleEngine procesed.");
  }while(st.status == scag::re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

  if(rcnt >= MAX_REDIRECT_CNT)
  {
    smsc_log_info(log,"DataSm: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
        src->getSystemId(), sms.getDestinationAddress().toString().c_str());
    DataResp(cmd,smsc::system::Status::NOROUTE);
    scag::sessions::SessionManager::Instance().releaseSession(session);                  
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }
  
  if(st.status != scag::re::STATUS_OK)
  {
    smsc_log_info(log,"DataSm: RuleEngine returned result=%d",st.result);
    if(!st.result)
    {
        smsc_log_warn(log, "DataSm: Rule failed and no error(zero rezult) returned");        
        st.result = smsc::system::Status::SYSERR;
    }
    DataResp(cmd, st.result);        
    scag::sessions::SessionManager::Instance().releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  try{
    int newSeq=dst->getNextSeq();
    if (!reg.Register(dst->getUid(),newSeq,cmd))
    {
      throw Exception("Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
    }
    stripUnknownSmppOptionals(sms,allowedUnknownOptionals);
    dst->putCommand(cmd);
    registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
  } catch(std::exception& e) {
    smsc_log_info(log,"DataSm: Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
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
  SmsCommand& smscmd=orgCmd->get_smsCommand();
  SMS* sms=orgCmd->get_sms();
  cmd->get_resp()->set_sms(sms);
  cmd->get_resp()->set_dir(smscmd.dir);

  cmd->set_serviceId(orgCmd->get_serviceId());
  cmd->set_operationId(orgCmd->get_operationId());
  sms->setOriginatingAddress(smscmd.orgSrc);
  sms->setDestinationAddress(smscmd.orgDst);

  scag::sessions::CSessionKey key;
  key.abonentAddr = (smscmd.dir == dsdSrv2Sc || smscmd.dir == dsdSc2Sc) ?
         orgCmd->get_sms()->getDestinationAddress():
         orgCmd->get_sms()->getOriginatingAddress();
  key.USR=orgCmd->get_sms()->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);

//  if(cmd.status != CMD_OK)
//      Statistics::Instance().registerEvent(SmppStatEvent(src->info, cntRejected, -1));

  scag::sessions::SessionPtr session=scag::sessions::SessionManager::Instance().getSession(key);
  if(!session.Get())
  {
    smsc_log_warn(log,"Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::TRANSACTIONTIMEDOUT);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
    smsc_log_debug(log, "datasm resp: processing...");
    st=scag::re::RuleEngine::Instance().process(cmd,*session);
    smsc_log_debug(log, "datasm resp: procesed.");
    if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "datasm resp: Rule failed and no error(zero rezult) returned");        
            st.result = smsc::system::Status::SYSERR;
        }
       
        rs = st.result;
        cmd->get_resp()->set_status(st.result);
    }
  }

  cmd->set_dialogId(orgCmd->get_dialogId());
  cmd->get_resp()->set_messageId("");

  try{
    dst->putCommand(cmd);
  } catch(std::exception& e) {
    smsc_log_warn(log,"DataSmResp:Failed to put command into %s:%s",dst->getSystemId(),e.what());
    rs = -1;
  }

  if(rs != -2 || st.status != scag::re::STATUS_OK)
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
        SmppCommand resp=SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT);
        resp.setEntity(routeMan->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
        processDeliveryResp(resp);
      }break;
      case SUBMIT:
      {
        SmppCommand resp=SmppCommand::makeSubmitSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT,false);
        resp.setEntity(routeMan->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
        processSubmitResp(resp);
      }break;
      case DATASM:
      {
        SmppCommand resp=SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT);
        resp.setEntity(routeMan->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
        processDataSmResp(resp);
      }break;
    }
  }
}

}//smpp
}//transport
}//scag
