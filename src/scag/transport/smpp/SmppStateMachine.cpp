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
using namespace scag::lcm;
using namespace scag::sessions;

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
    bool expired;
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
    lv.expired=false;
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
    if (!ptr)
    {
      return false;
    }
    cmd = ptr->cmd;
    cmd->set_dialogId(ptr->dlgId);
    toList.erase(ptr->it);
    reg.Delete(key);
    return true;
  }

  bool getExpiredCmd(SmppCommand& cmd,int &uid)
  {
    sync::MutexGuard mg(mtx);
    if (toList.empty()) return false;
    time_t now = time(NULL);
    if ((now - toList.front().insTime) < timeout) return false;
    if(toList.front().expired)
    {
      reg.Delete(toList.begin()->key);
      toList.erase(toList.begin());
      return false;
    }
    RegKey key = toList.front().key;
    RegValue* ptr = reg.GetPtr(key);
    if (!ptr)
    {
       // toList cleanup (find listValue by key & delete it)?
       toList.erase(toList.begin());
       return false;
    }
    toList.front().expired=true;
    cmd = ptr->cmd;
    cmd->set_dialogId(key.seq);
    uid=key.uid;
    return true;
  }

};

StateMachine::ResponseRegistry StateMachine::reg;

sync::Mutex StateMachine::expMtx;
bool StateMachine::expProc=false;


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

bool StateMachine::makeLongCall(SmppCommand& cx, SessionPtr& session)
{
    SmppCommand* cmd = new SmppCommand(cx);
    LongCallContext& lcmCtx = cx.getLongCallContext();
    lcmCtx.stateMachineContext = cmd;
    cmd->setSession(session);

    return LongCallManager::Instance().call(&lcmCtx);
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
    smsc_log_debug(log, "Submit: got %s", cmd.getLongCallContext().continueExec ? "continued..." : "");
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
    scag::sessions::SessionManager& sm = scag::sessions::SessionManager::Instance();

    smscmd.dir = dsdSrv2Sc;
    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src=cmd.getEntity();

    do{
      dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
      if(!dst || routeId == ri.routeId)
      {
        smsc_log_info(log,"Submit: %s %s(%s)->%s", !dst ? "no route" : "redirection to the same route",
        sms.getOriginatingAddress().toString().c_str(), src->getSystemId(),
        sms.getDestinationAddress().toString().c_str());
        SubmitResp(cmd,smsc::system::Status::NOROUTE);
        if(session.Get())
        {
            session->closeCurrentOperation();
            sm.releaseSession(session);
        }
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        return;
      }
      routeId = ri.routeId;
      cmd.setDstEntity(dst);

      sms.setRouteId(ri.routeId);
      sms.setSourceSmeId(src->getSystemId());
      sms.setDestinationSmeId(dst->getSystemId());

      cmd->set_serviceId(ri.serviceId);

      int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
                sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
      int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                    sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

      smsc_log_debug(log, "Submit: %s UMR=%d, USSD_OP=%d. %s(%s)->%s", rcnt ? "(redirected)" : "", umr, ussd_op,
        sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str());

      key.abonentAddr=sms.getDestinationAddress();
      if (ussd_op < 0) // SMPP, No USSD specific flags
      {
        if(!session.Get())
        {
            if(cmd.hasSession())
            {
                session = cmd.getSession();
                smsc_log_debug(log, "Submit: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
            }
            else if (umr < 0) {
                key.USR = 0;
                session=sm.newSession(key);
                sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
            }
            else {
                key.USR = umr;
                smsc_log_debug(log, "SMPP Submit: Continue, UMR=%d", umr);
                if(!sm.getSession(key, session, cmd))
                    return;
                if (!session.Get()) {
                    session=sm.newSession(key);
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
            session->closeCurrentOperation();
            sm.releaseSession(session);
            registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
            return;
        }

        if (umr < 0)
        {
            if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_REQUEST) { // New service USSD dialog
                smsc_log_debug(log, "USSD Submit: Begin service dialog...");
                session=sm.newSession(key);
                dst->setMapping(key.abonentAddr, 0, key.USR);
                smsc_log_debug(log, "USSD Submit: Created session USR=%d", key.USR);
                cmd->setFlag(SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG);
            }
            else {
                smsc_log_warn(log, "USSD Submit: UMR is not specified");
                SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, -1);
                return;
            }
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_REQUEST) { // Continue USSD dialog (or service initiated USSD by pending)
            key.USR = umr; umr = dst->getUMR(key.abonentAddr, key.USR);
            if(!sm.getSession(key, session, cmd))
                return;
            if (!session.Get()) {
                smsc_log_warn(log, "USSD Submit: USR=%d is invalid, no session", key.USR);
                SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
            if(umr <= 0) { // no mapping USR->UMR, service initiated USSD by pending
                smsc_log_debug(log, "USSD Submit: Begin service dialog(pending)...");
                dst->setMapping(key.abonentAddr, 0, key.USR);
                sms.dropProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
                cmd->setFlag(SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG);
            }
            else
            {
                smsc_log_debug(log, "USSD Submit: Continue, UMR=%d", umr);
                sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, umr);
            }
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_RESPONSE) { // End user USSD dialog
            key.USR = umr; umr = dst->getUMR(key.abonentAddr, key.USR);
            smsc_log_debug(log, "USSD Submit: End user dialog, UMR=%d", umr);
            if(!sm.getSession(key, session, cmd))
                return;
            if (!session.Get()) {
                smsc_log_warn(log, "USSD Submit: USR=%d is invalid, no session", key.USR);
                SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
            sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, umr);
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSN_REQUEST) { // End service USSD dialog
            key.USR = umr; umr = dst->getUMR(key.abonentAddr, key.USR);
            smsc_log_debug(log, "USSD Submit: End service dialog, UMR=%d", umr);
            if(!sm.getSession(key, session, cmd))
                return;
            if (!session.Get()) {
                smsc_log_warn(log, "USSD Submit: USR=%d is invalid, no session", key.USR);
                SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
            sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, umr);
//            dst->delUMRMapping(key.abonentAddr, umr);
//            dst->delUMRMapping(key.abonentAddr, 0);
        }
        else {
            smsc_log_warn(log, "USSD Submit: USSD_OP=%d is invalid", ussd_op);
            SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM); // TODO: status
            registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
            return;
        }
      }

      smsc_log_debug(log, "Submit: RuleEngine processing...");
      scag::re::RuleEngine::Instance().process(cmd,*session, st);
      smsc_log_debug(log, "Submit: RuleEngine procesed.");

  }while(st.status == scag::re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

  if(rcnt >= MAX_REDIRECT_CNT)
  {
    smsc_log_info(log,"Submit: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
        src->getSystemId(), sms.getDestinationAddress().toString().c_str());
    SubmitResp(cmd,smsc::system::Status::NOROUTE);
    session->closeCurrentOperation();
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }

  if(st.status == scag::re::STATUS_LONG_CALL)
  {
    smsc_log_info(log,"Submit: long call initiate");
    makeLongCall(cmd, session);
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
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  try{
      if(dst->getBindType() == btNone)
      {
        smsc_log_info(log,"Submit: sme not connected %s(%s)->%s(%s)", sms.getOriginatingAddress().toString().c_str(), src->getSystemId(),
            sms.getDestinationAddress().toString().c_str(), dst->getSystemId());
        SubmitResp(cmd,smsc::system::Status::SMENOTCONNECTED);
        session->closeCurrentOperation();        
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::SMENOTCONNECTED);
      }
      else
      {
        int newSeq=dst->getNextSeq();
        if (!reg.Register(dst->getUid(), newSeq, cmd))
        {
          throw Exception("Submit: Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
        }
        stripUnknownSmppOptionals(sms,allowedUnknownOptionals);

        dst->putCommand(cmd);
        registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
      }
  } catch(std::exception& e) {
    SubmitResp(cmd,smsc::system::Status::SYSFAILURE);
    registerEvent(scag::stat::events::smpp::FAILED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSFAILURE);
    session->closeCurrentOperation();
    smsc_log_info(log,"Submit: Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
  }

  st.runPostProcessActions();

  sm.releaseSession(session);
  smsc_log_debug(log, "Submit: processed");
}


void StateMachine::processSubmitResp(SmppCommand& cmd)
{
  int rs = -2;
  scag::re::RuleStatus st;

  SmppEntity *dst, *src;
  scag::sessions::SessionPtr session;
  scag::sessions::CSessionKey key;
  int ussd_op;
  SMS *sms;
  SmppCommand orgCmd;

  src=cmd.getEntity();

  if(!cmd.getLongCallContext().continueExec)
  {      
      smsc_log_debug(log, "SubmitResp: got");

      int srcUid = 0;
      if(cmd->get_resp()->expiredResp)
      {
        srcUid=cmd->get_resp()->expiredUid;
      }else
      {
        try { srcUid = src->getUid(); }
        catch (std::exception& exc) {
          smsc_log_warn(log, "SubmitResp: Src entity disconnected. sid='%s', seq='%d'",
                        src->getSystemId(), cmd->get_dialogId());
          registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
          return;
        }
      }
      if(!reg.Get(srcUid, cmd->get_dialogId(), orgCmd)) {
        smsc_log_warn(log,"SubmitResp: Original submit for submit response not found. sid='%s',seq='%d'",
                      src->getSystemId(),cmd->get_dialogId());
        registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
        return;
      }

      dst=orgCmd.getEntity();
      cmd.setDstEntity(dst);
      sms=orgCmd->get_sms();

      cmd->get_resp()->set_sms(sms);
      cmd->set_serviceId(orgCmd->get_serviceId());
      cmd->set_operationId(orgCmd->get_operationId());
      cmd->get_resp()->set_dir(dsdSc2Srv);

      sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
      sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);

      cmd->get_resp()->setOrgCmd(orgCmd);
  }
  else
  {
      smsc_log_debug(log, "SubmitResp: continued...");
      dst = cmd.getDstEntity();
      sms = cmd->get_resp()->get_sms();
  }

  ussd_op = sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ? sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

  if(cmd.hasSession())
  {
    session = cmd.getSession();
    smsc_log_debug(log, "SubmitResp: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
  }
  else
  {
      key.abonentAddr=sms->getDestinationAddress();
      int umr = sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
      key.USR = (ussd_op < 0) ? umr : src->getUSR(key.abonentAddr, umr);

      if(!scag::sessions::SessionManager::Instance().getSession(key, session, cmd))
      {
          cmd.getLongCallContext().continueExec = true;
          return;
      }
      smsc_log_warn(log,"SubmitResp: got session. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
   }


  if(!session.Get())
  {
    smsc_log_warn(log,"SubmitResp: Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::TRANSACTIONTIMEDOUT);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
    smsc_log_debug(log, "SubmitResp: RuleEngine processing...");
    scag::re::RuleEngine::Instance().process(cmd,*session, st);
    smsc_log_debug(log, "SubmitResp: RuleEngine  processed");

      if(st.status == scag::re::STATUS_LONG_CALL)
      {
          smsc_log_debug(log,"SubmitResp: long call initiate");
          makeLongCall(cmd, session);
          return;
      }

    if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "SubmitResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }

        rs = st.result;
        cmd->get_resp()->set_status(st.result);
    }
  }

  if (ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_RESPONSE ||
      ussd_op == smsc::smpp::UssdServiceOpValue::USSN_REQUEST)
      src->delUSRMapping(key.abonentAddr, key.USR); // Empty mapping on USSD dialog end

//  cmd->set_dialogId(orgCmd->get_dialogId());
  if (!(cmd->get_resp()->get_messageId())) {
    smsc_log_warn(log, "SubmitResp: messageId is null");
    cmd->get_resp()->set_messageId("");
  }

  try {
      dst->putCommand(cmd);
  } catch(std::exception& e) {
    smsc_log_warn(log,"SubmitResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
    rs = -1;
  }

  st.runPostProcessActions();

  if(rs != -2 || st.status != scag::re::STATUS_OK)
    registerEvent(scag::stat::events::smpp::RESP_FAILED, src, dst, (char*)sms->getRouteId(), rs);
  else
    registerEvent(scag::stat::events::smpp::RESP_OK, src, dst, (char*)sms->getRouteId(), -1);

  if(session.Get())scag::sessions::SessionManager::Instance().releaseSession(session);
  cmd->get_resp()->set_sms(0);
  smsc_log_debug(log, "SubmitResp: processed");
}

void StateMachine::sendReceipt(SmppCommand& cmd)
{
  SmppEntity *dst = cmd.getDstEntity();
  try{
      if(dst->getBindType() == btNone)
        smsc_log_info(log,"MSAG Receipt: sme not connected (%s)", dst->getSystemId());
      else
      {
        int newSeq=dst->getNextSeq();
        if(!reg.Register(dst->getUid(), newSeq, cmd))
          throw Exception("MSAG Receipt: Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
        dst->putCommand(cmd);
      }
  } catch(std::exception& e) {
    smsc_log_info(log, "MSAG Receipt: Failed to putCommand into %s:%s", dst->getSystemId(), e.what());
  }
}

void StateMachine::processDelivery(SmppCommand& cmd)
{
    smsc_log_debug(log, "Delivery: got %s", cmd.getLongCallContext().continueExec ? "continued..." : "");
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
    scag::sessions::SessionManager& sm = scag::sessions::SessionManager::Instance();

    if(cmd->flagSet(SmppCommandFlags::NOTIFICATION_RECEIPT)) {
        sendReceipt(cmd);
        return;
    }
    
    smscmd.dir = dsdSc2Srv;
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
        {
            session->closeCurrentOperation();
            sm.releaseSession(session);
        }
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        return;
      }
      routeId = ri.routeId;
      cmd.setDstEntity(dst);

      sms.setRouteId(ri.routeId);
      sms.setSourceSmeId(src->getSystemId());
      sms.setDestinationSmeId(dst->getSystemId());

      cmd->set_serviceId(ri.serviceId);

      int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
                sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
      int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                    sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

      smsc_log_debug(log, "Delivery: %s UMR=%d, USSD_OP=%d. %s(%s)->%s", rcnt ? "(redirected)" : "", umr, ussd_op,
        sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str());

      key.abonentAddr=sms.getOriginatingAddress();
      if (ussd_op < 0) // SMPP, No USSD specific flags
      {
          if(!session.Get())
          {
            if(cmd.hasSession())
            {
                session = cmd.getSession();
                smsc_log_debug(log, "Delivery: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
            }
            else
              {
                  key.USR = 0; // Always create new session
                  session=sm.newSession(key);
                  sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
                  if (umr > 0) {
                      smsc_log_warn(log, "USSD Delivery: UMR=%d is set. "
                                         "Created new session instead. USR=%d", umr, key.USR);
                    umr = -1;
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
                smsc_log_warn(log, "USSD Delivery: Rerouting for USSD dialog not allowed");
                DeliveryResp(cmd,smsc::system::Status::NOROUTE);
                session->closeCurrentOperation();
                sm.releaseSession(session);
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
              session=sm.newSession(key);
              sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
              smsc_log_debug(log, "USSD Delivery: Creating mapping USR=%d, UMR=%d", key.USR, umr);
              if (src) src->setMapping(key.abonentAddr, umr, key.USR);
          }
          else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_CONFIRM) { // Continue USSD dialog
                int tmpUSR = src->getUSR(key.abonentAddr, umr);
                if (tmpUSR == -1) {
                    tmpUSR = src->getUSR(key.abonentAddr, 0);
                    if(tmpUSR == -1) smsc_log_warn(log, "USSD Delivery: USR mapping not found");
                    src->delUMRMapping(key.abonentAddr, 0);
                    src->setMapping(key.abonentAddr, umr, tmpUSR);
                }
                key.USR = tmpUSR;
                smsc_log_debug(log, "USSD Delivery: Continue USR=%d", key.USR);
                if(!sm.getSession(key, session, cmd))
                    return;
                if (!session.Get()) {
                    smsc_log_warn(log, "USSD Delivery: USR=%d is invalid, no session", key.USR);
                    DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                    return;
                }
                sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
          }
          else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSN_CONFIRM) { // End service USSD dialog
                int tmpUSR = src->getUSR(key.abonentAddr, umr);
                if (tmpUSR == -1) {
                    tmpUSR = src->getUSR(key.abonentAddr, 0);
                    if(tmpUSR == -1) smsc_log_warn(log, "USSD Delivery: USR mapping not found");
                }
                src->delUMRMapping(key.abonentAddr, umr);
                src->delUMRMapping(key.abonentAddr, 0); // For service initiated USSD dialog
                key.USR = tmpUSR;
                smsc_log_debug(log, "USSD Delivery: End service dialog, USR=%d", key.USR);
                if(!sm.getSession(key, session, cmd))
                    return;
                if (!session.Get()) {
                    smsc_log_warn(log, "USSD Delivery: USR=%d is invalid, no session", key.USR);
                    DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                    return;
                }
          }
          else {
              smsc_log_warn(log, "USSD Delivery: USSD_OP=%d is invalid", ussd_op);
              DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM);
              registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
              return;
          }
      }

      smsc_log_debug(log, "Delivery: RuleEngine processing...");
      scag::re::RuleEngine::Instance().process(cmd,*session, st);
      smsc_log_debug(log, "Delivery: RuleEngine procesed.");

  }while(st.status == scag::re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

  if(rcnt >= MAX_REDIRECT_CNT)
  {
    smsc_log_info(log,"Delivery: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s",  sms.getOriginatingAddress().toString().c_str(),
          src->getSystemId(), sms.getDestinationAddress().toString().c_str());

    DeliveryResp(cmd,smsc::system::Status::NOROUTE);
    session->closeCurrentOperation();
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }

  if(st.status == scag::re::STATUS_LONG_CALL)
  {
      smsc_log_debug(log,"Delivery: long call initiate");
      makeLongCall(cmd, session);
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
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  try{
      if(dst->getBindType() == btNone)
      {
        smsc_log_info(log,"Delivery: sme not connected %s(%s)->%s(%s)", sms.getOriginatingAddress().toString().c_str(), src->getSystemId(),
            sms.getDestinationAddress().toString().c_str(), dst->getSystemId());
        DeliveryResp(cmd,smsc::system::Status::SMENOTCONNECTED);
        session->closeCurrentOperation();        
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::SMENOTCONNECTED);
      }
      else
      {
        int newSeq=dst->getNextSeq();
        if (!reg.Register(dst->getUid(),newSeq,cmd))
        {
          throw Exception("Delivery: Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
        }
        stripUnknownSmppOptionals(sms,allowedUnknownOptionals);

        dst->putCommand(cmd);
        registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
      }

  } catch(std::exception& e) {
    smsc_log_info(log,"Delivery: Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
    DeliveryResp(cmd,smsc::system::Status::SYSFAILURE);
    session->closeCurrentOperation();    
    registerEvent(scag::stat::events::smpp::FAILED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSFAILURE);
  }

  st.runPostProcessActions();

  sm.releaseSession(session);
  smsc_log_debug(log, "Delivery: processed");
}

void StateMachine::processDeliveryResp(SmppCommand& cmd)
{
  int rs = -2;
  scag::re::RuleStatus st;
  SmppEntity *dst, *src;
  SMS* sms;

  scag::sessions::SessionPtr session;
  scag::sessions::CSessionKey key;

  src=cmd.getEntity();

  if(!cmd.getLongCallContext().continueExec)
  {
      smsc_log_debug(log, "DeliveryResp: got");

      SmppCommand orgCmd;
      int srcUid = 0;
      if(cmd->get_resp()->expiredResp) {
        srcUid=cmd->get_resp()->expiredUid;
      } else  {
        try { srcUid = src->getUid(); }
        catch (std::exception& exc) {
          smsc_log_warn(log, "DeliveryResp: Src entity disconnected. sid='%s', seq='%d'",
                        src->getSystemId(), cmd->get_dialogId());
          registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
          return;
        }
      }
      bool bGotFromRegistry = reg.Get(srcUid, cmd->get_dialogId(), orgCmd);
      if(bGotFromRegistry && orgCmd->flagSet(SmppCommandFlags::NOTIFICATION_RECEIPT)) {
        smsc_log_debug(log, "MSAG Receipt: Got responce, skipped");
        return;
      }
      if(!bGotFromRegistry) {
        smsc_log_warn(log,"DeliveryResp: Original delivery for delivery response not found. sid='%s',seq='%d'",
              src->getSystemId(),cmd->get_dialogId());
        registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
        return;
      }

      dst=orgCmd.getEntity();
      cmd.setDstEntity(dst);
      sms=orgCmd->get_sms();
      smsc_log_debug(log, "sms:%x sm: %d mp: %d rsm: %d rmp: %d", sms, sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE), sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD), sms->hasBinProperty(Tag::SMSC_RAW_SHORTMESSAGE), sms->hasBinProperty(Tag::SMSC_RAW_PAYLOAD));
      cmd->get_resp()->set_sms(sms);
      cmd->set_serviceId(orgCmd->get_serviceId());
      cmd->set_operationId(orgCmd->get_operationId());
      cmd->get_resp()->set_dir(dsdSrv2Sc);

      sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
      sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);

      cmd->get_resp()->setOrgCmd(orgCmd);
  }
  else
  {
    smsc_log_debug(log, "DeliveryResp: continued...");
    dst = cmd.getDstEntity();
    sms = cmd->get_resp()->get_sms();
  }

    if(cmd.hasSession())
    {
        session = cmd.getSession();
        smsc_log_debug(log, "DeliveryResp: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
    }
    else
    {
        key.abonentAddr = sms->getOriginatingAddress();
        key.USR = sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);

        if(!scag::sessions::SessionManager::Instance().getSession(key, session, cmd))
        {
          cmd.getLongCallContext().continueExec = true;
          return;
        }
        smsc_log_warn(log,"DeliveryResp: got session. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    }


  if(!session.Get())
  {
    smsc_log_warn(log,"DeliveryResp: Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::TRANSACTIONTIMEDOUT);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
    smsc_log_debug(log, "sms:%x sm: %d mp: %d rsm: %d rmp: %d", sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE), sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD), sms->hasBinProperty(Tag::SMSC_RAW_SHORTMESSAGE), sms->hasBinProperty(Tag::SMSC_RAW_PAYLOAD));
    smsc_log_debug(log, "DeliveryResp: processing...");
    scag::re::RuleEngine::Instance().process(cmd,*session, st);
    smsc_log_debug(log, "DeliveryResp: procesed.");

      if(st.status == scag::re::STATUS_LONG_CALL)
      {
//          if (!reg.Register(srcUid, cmd->get_dialogId(), orgCmd))
//            throw Exception("DeliveryResp: Register cmd for uid=%d, seq=%d failed", dst->getUid(), cmd->get_dialogId());

          smsc_log_debug(log,"DeliveryResp: long call initiate");
          makeLongCall(cmd, session);
          return;
      }

    if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "DeliveryResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }

        rs = st.result;
        cmd->get_resp()->set_status(st.result);
    }
  }

//  cmd->set_dialogId(orgCmd->get_dialogId());
  cmd->get_resp()->set_messageId("");

  try{
    dst->putCommand(cmd);
  } catch(std::exception& e) {
    smsc_log_warn(log,"DeliveryResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
    rs = -1;
  }

  st.runPostProcessActions();

  if(rs != -2 || st.status != scag::re::STATUS_OK)
      registerEvent(scag::stat::events::smpp::RESP_FAILED, src, dst, (char*)sms->getRouteId(), rs);
  else
      registerEvent(scag::stat::events::smpp::RESP_OK, src, dst, (char*)sms->getRouteId(), -1);

  if(session.Get())scag::sessions::SessionManager::Instance().releaseSession(session);
  cmd->get_resp()->set_sms(0);
  smsc_log_debug(log, "DeliveryResp: processed");
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
    smsc_log_debug(log, "DataSm: got %s", cmd.getLongCallContext().continueExec ? "continued..." : "");
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
    scag::sessions::SessionManager& sm = scag::sessions::SessionManager::Instance();

    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src=cmd.getEntity();

    if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
    {
        smsc_log_info(log,"DataSm: USSD dialog not allowed in DataSm %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
                        src->getSystemId(), sms.getDestinationAddress().toString().c_str());
        DataResp(cmd,smsc::system::Status::USSDDLGREFMISM);
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::USSDDLGREFMISM);
    }

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
        {
            session->closeCurrentOperation();
            sm.releaseSession(session);
        }
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        return;
      }
      routeId = ri.routeId;
      cmd.setDstEntity(dst);

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

      smsc_log_debug(log, "DataSm: %s UMR=%d. %s(%s)->%s", rcnt ? "(redirected)" : "", umr,
                sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str());

      key.abonentAddr = (src->info.type == etService) ? sms.getDestinationAddress() : sms.getOriginatingAddress();
        if(!session.Get())
        {
          if(cmd.hasSession())
          {
              session = cmd.getSession();
              smsc_log_debug(log, "DataSm: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
          }
          else if (umr < 0) {
              key.USR = 0;
              session=sm.newSession(key);
              sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
          }
          else {
              key.USR = umr;
              smsc_log_debug(log, "DataSm: Continue, UMR=%d", umr);
              if(!sm.getSession(key, session, cmd))
                  return;
              if (!session.Get()) {
                  session=sm.newSession(key);
                  sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
                  smsc_log_warn(log, "DataSm: Session for USR=%d not found, created new USR=%d", umr, key.USR);
              }
          }
        }
        else
            session->setRedirectFlag();
      smsc_log_debug(log, "DataSm: RuleEngine processing...");
      scag::re::RuleEngine::Instance().process(cmd,*session, st);
      smsc_log_debug(log, "DataSm: RuleEngine procesed.");
  }while(st.status == scag::re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

  if(rcnt >= MAX_REDIRECT_CNT)
  {
    smsc_log_info(log,"DataSm: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
        src->getSystemId(), sms.getDestinationAddress().toString().c_str());
    DataResp(cmd,smsc::system::Status::NOROUTE);
    session->closeCurrentOperation();
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
    return;
  }

  if(st.status == scag::re::STATUS_LONG_CALL)
  {
      smsc_log_debug(log,"DataSm: long call initiate");
      makeLongCall(cmd, session);
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
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  try{
      if(dst->getBindType() == btNone)
      {
        smsc_log_info(log,"DataSm: sme not connected %s(%s)->%s(%s)", sms.getOriginatingAddress().toString().c_str(), src->getSystemId(),
            sms.getDestinationAddress().toString().c_str(), dst->getSystemId());
        DataResp(cmd,smsc::system::Status::SMENOTCONNECTED);
        session->closeCurrentOperation();        
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::SMENOTCONNECTED);
      }
      else
      {
        int newSeq=dst->getNextSeq();
        if (!reg.Register(dst->getUid(),newSeq,cmd))
        {
          throw Exception("Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
        }
        stripUnknownSmppOptionals(sms,allowedUnknownOptionals);

        dst->putCommand(cmd);
        registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
      }
  } catch(std::exception& e) {
    smsc_log_info(log,"DataSm: Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
    DataResp(cmd,smsc::system::Status::SYSFAILURE);
    session->closeCurrentOperation();
    registerEvent(scag::stat::events::smpp::FAILED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSFAILURE);
  }

  st.runPostProcessActions();

  sm.releaseSession(session);

  smsc_log_debug(log, "DataSm: processed");
}

void StateMachine::processDataSmResp(SmppCommand& cmd)
{
  int rs = -2;
  scag::re::RuleStatus st;

  scag::sessions::CSessionKey key;
  scag::sessions::SessionPtr session;
  SmppEntity *dst, *src;
  SMS* sms;

  src = cmd.getEntity();

  if(!cmd.getLongCallContext().continueExec)
  {
      smsc_log_debug(log, "DataSmResp: got");

      SmppCommand orgCmd;
      int srcUid = 0;
      if(cmd->get_resp()->expiredResp)
      {
        srcUid=cmd->get_resp()->expiredUid;
      }else
      {
        try { srcUid = src->getUid(); }
        catch (std::exception& exc) {
          smsc_log_warn(log, "DataSmResp: Src entity disconnected. sid='%s', seq='%d'",
                        src->getSystemId(), cmd->get_dialogId());
          registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
          return;
        }
      }
      if(!reg.Get(srcUid, cmd->get_dialogId(), orgCmd)) {
        smsc_log_warn(log,"DataSmResp: Original datasm for datasm response not found. sid='%s',seq='%d'",src->getSystemId(),cmd->get_dialogId());
        registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
        return;
      }

      SmsCommand& smscmd=orgCmd->get_smsCommand();
      sms=orgCmd->get_sms();
      cmd->get_resp()->set_sms(sms);

      if (smscmd.dir == dsdSrv2Sc) 
          cmd->get_resp()->set_dir(dsdSc2Srv);
      else if (smscmd.dir == dsdSc2Srv) 
          cmd->get_resp()->set_dir(dsdSrv2Sc);
      else
          cmd->get_resp()->set_dir(smscmd.dir);

      cmd->set_serviceId(orgCmd->get_serviceId());
      cmd->set_operationId(orgCmd->get_operationId());
      sms->setOriginatingAddress(smscmd.orgSrc);
      sms->setDestinationAddress(smscmd.orgDst);

      dst=orgCmd.getEntity();
      cmd.setDstEntity(dst);

      cmd->get_resp()->setOrgCmd(orgCmd);
  }
  else
  {
    smsc_log_debug(log, "DataSmResp: continued...");
    dst = cmd.getDstEntity();
    sms = cmd->get_resp()->get_sms();
  }

  if(cmd.hasSession())
  {
      session = cmd.getSession();
      smsc_log_debug(log, "DataSmResp: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
  }
  else
  {
      key.abonentAddr = (cmd->get_resp()->get_dir() == dsdSc2Srv || cmd->get_resp()->get_dir() == dsdSrv2Srv) ?
             sms->getDestinationAddress() : sms->getOriginatingAddress();
      key.USR=sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);

      if(!scag::sessions::SessionManager::Instance().getSession(key, session, cmd))
      {
          cmd.getLongCallContext().continueExec = true;
          return;
      }
      smsc_log_warn(log,"DataSmResp: got session. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
  }

  if(!session.Get())
  {
    smsc_log_warn(log,"Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::TRANSACTIONTIMEDOUT);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
    smsc_log_debug(log, "DataSmResp: processing...");
    scag::re::RuleEngine::Instance().process(cmd,*session, st);
    smsc_log_debug(log, "DataSmResp: procesed.");

      if(st.status == scag::re::STATUS_LONG_CALL)
      {
          smsc_log_debug(log,"DataSmResp: long call initiate");
          makeLongCall(cmd, session);
          return;
      }

    if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "DataSmResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }

        rs = st.result;
        cmd->get_resp()->set_status(st.result);
    }
  }

//  cmd->set_dialogId(orgCmd->get_dialogId());
  if (!(cmd->get_resp()->get_messageId())) {
    cmd->get_resp()->set_messageId("");
  }
  

  try{
    dst->putCommand(cmd);
  } catch(std::exception& e) {
    smsc_log_warn(log,"DataSmResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
    rs = -1;
  }
  
  st.runPostProcessActions();

  if(rs != -2 || st.status != scag::re::STATUS_OK)
      registerEvent(scag::stat::events::smpp::RESP_FAILED, src, dst, (char*)sms->getRouteId(), rs);
  else
      registerEvent(scag::stat::events::smpp::RESP_OK, src, dst, (char*)sms->getRouteId(), -1);

  scag::sessions::SessionManager::Instance().releaseSession(session);

  cmd->get_resp()->set_sms(0);

  smsc_log_debug(log, "DataSmResp: processed");
}

void StateMachine::processExpiredResps()
{
  {
    MutexGuard mg(expMtx);
    if(expProc)return;
    expProc=true;
  }
  SmppCommand cmd;
  int uid;
  while(reg.getExpiredCmd(cmd,uid))
  {
    switch(cmd->get_commandId())
    {
      case DELIVERY:
      {
        SmppCommand resp=SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT);
        resp->get_resp()->expiredResp=true;
        resp->get_resp()->expiredUid=uid;
        resp.setEntity(routeMan->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
        processDeliveryResp(resp);
      }break;
      case SUBMIT:
      {
        SmppCommand resp=SmppCommand::makeSubmitSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT,false);
        resp->get_resp()->expiredResp=true;
        resp->get_resp()->expiredUid=uid;
        resp.setEntity(routeMan->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
        processSubmitResp(resp);
      }break;
      case DATASM:
      {
        SmppCommand resp=SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT);
        resp->get_resp()->expiredResp=true;
        resp->get_resp()->expiredUid=uid;
        resp.setEntity(routeMan->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
        processDataSmResp(resp);
      }break;
    }
  }
  {
    MutexGuard mg(expMtx);
    expProc=false;
  }
}

}//smpp
}//transport
}//scag
