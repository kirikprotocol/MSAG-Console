#include "SmppStateMachine.h"
#include "core/buffers/XHash.hpp"
#include "scag/re/RuleEngine.h"
#include "scag/sessions/SessionManager.h"
#include "scag/stat/StatisticsManager.h"
#include "system/status.h"
#include "SmppUtil.h"
#include "SmppManager.h"
#include "SmsSplit.h"

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
    RegKey(int _uid=-1, int _seq=-1):uid(_uid), seq(_seq) {}
    bool operator==(const RegKey& key)
    {
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
    TimeOutList::iterator it;
  };
  struct HashFunc{
    static unsigned int CalcHash(const RegKey& key) {
      return key.uid<<9+key.seq;
    }
  };

  buf::XHash<RegKey, RegValue, HashFunc> reg;
  buf::Hash<int> outCnt;
  sync::Mutex mtx;

  bool Register(int uid, int seq, SmppCommand& cmd, bool limit = true)
  {
    RegKey key(uid, seq);
    if (!log) log=smsc::logger::Logger::getInstance("respreg");
    sync::MutexGuard mg(mtx);
    if (reg.Exists(key))
    { // key already registered
        smsc_log_warn(log, "register %d/%d failed", uid, seq);
        return false;
    }
    if(cmd.getDstEntity()->info.outQueueLimit > 0)
    {
      int* p=outCnt.GetPtr(cmd.getDstEntity()->info.systemId.c_str());
      if(limit && p && *p>cmd.getDstEntity()->info.outQueueLimit)
      {
        smsc_log_warn(log,"command registration for '%s' denied: outQueueLimit",
                      cmd.getDstEntity()->info.systemId.c_str());
        return false;
      }
      if(p)
      {
        (*p)++;
      }else
      {
        outCnt.Insert(cmd.getDstEntity()->info.systemId.c_str(),1);
      }
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
    cmd->set_dialogId(seq);
    reg.Insert(key,val);
    return true;
  }

  bool Get(int uid, int seq, SmppCommand& cmd)
  {
    if (!log) log=smsc::logger::Logger::getInstance("respreg");
    sync::MutexGuard mg(mtx);
    RegKey key(uid, seq);
    RegValue* ptr=reg.GetPtr(key);
    smsc_log_debug(log, "get %d/%d - %s", uid, seq, (ptr) ? "ok":"not found");
    if (!ptr)
    {
      return false;
    }
    cmd = ptr->cmd;
    //cmd->set_dialogId(ptr->dlgId);
    toList.erase(ptr->it);
    reg.Delete(key);
    if(cmd.getDstEntity()->info.outQueueLimit > 0)
    {
      outCnt.Get(cmd.getDstEntity()->info.systemId.c_str())--;
    }
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
      RegValue* ptr=reg.GetPtr(toList.begin()->key);
      if(ptr && ptr->cmd.getDstEntity()->info.outQueueLimit > 0)
      {
        outCnt.Get(ptr->cmd.getDstEntity()->info.systemId.c_str())--;
      }
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
    cmd->setFlag(SmppCommandFlags::EXPIRED_COMMAND);
    //cmd->set_dialogId(key.seq);
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
      while(queue->getCommand(cmd))
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

    Statistics::Instance().registerEvent(SmppStatEvent(src_id, srcType, dst_id, dstType, rid, event, errCode));
}

uint32_t StateMachine::putCommand(CommandId cmdType, SmppEntity* src, SmppEntity* dst, router::RouteInfo& ri, SmppCommand& cmd)
{
  const char *cmdName = cmdType == SUBMIT ? "SubmitSm" : cmdType == DELIVERY ? "DeliverySm" : "DataSm";
  uint32_t failed = 0;
  SMS& sms = *cmd->get_sms();
  
  if(cmd.hasSession())
    cmd.setSession(SessionPtr(NULL)); // cmd can lay in out queue quite long time, so let's free session right now.
  
  try{
      if(dst->getBindType() == btNone)
      {
        smsc_log_info(log,"%s: sme not connected %s(%s)->%s(%s)", cmdName, sms.getOriginatingAddress().toString().c_str(), src->getSystemId(),
            sms.getDestinationAddress().toString().c_str(), dst->getSystemId());
        failed = smsc::system::Status::SMENOTCONNECTED;
      }
      else
      {
        stripUnknownSmppOptionals(sms,allowedUnknownOptionals);      

        uint32_t cnt;
        if(ri.slicing != SlicingType::NONE && !isSliced(sms) && (cnt = getPartsCount(sms)) > 1)
        {
            smsc_log_debug(log, "%s: slicing message, type=%d, parts=%d, resppolicy=%d", cmdName, ri.slicing, cnt, ri.slicingRespPolicy);
            cmd->setSlicingParams(ri.slicingRespPolicy, cnt);
            uint32_t seq = 0, refNum = dst->getNextSlicingSeq();

            SMS partSms;
            uint8_t udhType = ri.slicing > SlicingType::SAR ? ri.slicing - SlicingType::SAR : 0;
            while(seq = getNextSmsPart(sms, partSms, refNum, seq, udhType, cmdType == DATASM))
            {
                int newSeq=dst->getNextSeq();
                SmppCommand partCmd = SmppCommand::makeCommandSm(cmdType, partSms, newSeq);
                if (!reg.Register(dst->getUid(), newSeq, cmd, seq == 1))
                  throw Exception("%s: Register cmd for uid=%d, seq=%d failed", cmdName, dst->getUid(), newSeq);
                dst->putCommand(partCmd);
            }
        }
        else
        {
            int newSeq=dst->getNextSeq();
            if (!reg.Register(dst->getUid(), newSeq, cmd))
              throw Exception("%s: Register cmd for uid=%d, seq=%d failed", cmdName, dst->getUid(), newSeq);
            dst->putCommand(cmd);            
        }            
        
        registerEvent(scag::stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
      }
  } catch(std::exception& e)
  {
    failed = smsc::system::Status::SYSFAILURE;
    smsc_log_info(log,"%s: Failed to putCommand into %s:%s", cmdName, dst->getSystemId(),e.what());
  }
  
  return failed;
}

void StateMachine::processSubmit(SmppCommand& cmd)
{
    smsc_log_debug(log, "Submit: got %s", cmd.hasSession() ? "continued..." : "");
    uint32_t rcnt = 0, failed = 0;
    SmppEntity *src = NULL; SmppEntity *dst = NULL;
    bool bSibincoUSSD = true; int original_ussd_op = -1;
    scag::sessions::SessionPtr session;
    scag::sessions::CSessionKey key;
    router::RouteInfo ri;
    buf::FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;
    scag::re::RuleStatus st;
    SMS& sms=*cmd->get_sms();
    SmsCommand& smscmd=cmd->get_smsCommand();
    scag::sessions::SessionManager& sm = scag::sessions::SessionManager::Instance();

    int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
              sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
    int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                  sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;
    if (ussd_op == 35) // Not Sibinco USSD dialog
    {
        smsc_log_debug(log, "Submit: Not Sibinco USSD dialog for %s", 
                       sms.getDestinationAddress().toString().c_str());
        sms.dropProperty(Tag::SMPP_USSD_SERVICE_OP);
        original_ussd_op = ussd_op; ussd_op = -1; umr = -1; bSibincoUSSD = false;
    }

    smscmd.dir = dsdSrv2Sc;
    smscmd.orgSrc=sms.getOriginatingAddress();    smscmd.orgDst=sms.getDestinationAddress();
    src=cmd.getEntity();

    cmd->set_orgDialogId(cmd->get_dialogId());

    do
    {
      dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
      smsc_log_debug(log, "orig_route_id=%s, new_route_id=%s", routeId.c_str(), ri.routeId.c_str());
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

      smsc_log_info(log, "Submit %s: %s UMR=%d, USSD_OP=%d. %s(%s)->%s, routeId=%s", cmd.hasSession() ? "continued..." : "", rcnt ? "(redirected)" : "", umr, ussd_op,
        sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str(), routeId.c_str());

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
            else if (umr < 0)
            {
                key.USR = 0;
                session=sm.newSession(key);
                sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
            }
            else
            {
                key.USR = umr;
                smsc_log_debug(log, "SMPP Submit: Continue, UMR=%d", umr);
                if(!sm.getSession(key, session, cmd)) return;
                if (!session.Get())
                {
                    session=sm.newSession(key);
                    sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
                    smsc_log_warn(log, "SMPP Submit: Session for USR=%d not found, created new USR=%d, Address=%s", umr, key.USR, key.abonentAddr.toString().c_str());
                }
            }
        }
        else
        {
          session->getLongCallContext().continueExec = true;
        }
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

        if(cmd.hasSession())
        {
            session = cmd.getSession();
            smsc_log_debug(log, "Submit(USSD): session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
        }
        else if (umr < 0)
        {
            if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_REQUEST)
            { // New service USSD dialog
                smsc_log_debug(log, "USSD Submit: Begin service dialog...");
                session=sm.newSession(key);
                dst->setMapping(key.abonentAddr, 0, key.USR);
                smsc_log_debug(log, "USSD Submit: Created session USR=%d", key.USR);
                cmd->setFlag(SmppCommandFlags::SERVICE_INITIATED_USSD_DIALOG);
            }
            else
            {
                smsc_log_warn(log, "USSD Submit: UMR is not specified");
                SubmitResp(cmd, smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_REQUEST)
        { // Continue USSD dialog (or service initiated USSD by pending)
            key.USR = umr; umr = dst->getUMR(key.abonentAddr, key.USR);
            if(umr <= 0)
            { // no mapping USR->UMR, service initiated USSD by pending
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
            if(!sm.getSession(key, session, cmd)) return; // TODO: JOPA (was before if(umr <= 0)) !!!
            if (!session.Get())
            {
                smsc_log_warn(log, "USSD Submit: USR=%d is invalid, no session", key.USR);
                SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_RESPONSE)
        { // End user USSD dialog
            key.USR = umr; umr = dst->getUMR(key.abonentAddr, key.USR);
            smsc_log_debug(log, "USSD Submit: End user dialog, UMR=%d", umr);
            sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, umr);
            if(!sm.getSession(key, session, cmd))
            {
              return;
            }
            if (!session.Get())
            {
                smsc_log_warn(log, "USSD Submit: USR=%d is invalid, no session", key.USR);
                SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSN_REQUEST)
        { // End service USSD dialog
            key.USR = umr; umr = dst->getUMR(key.abonentAddr, key.USR);
            smsc_log_debug(log, "USSD Submit: End service dialog, UMR=%d", umr);
            sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, umr);
            if(!sm.getSession(key, session, cmd)) return;
            if (!session.Get())
            {
                smsc_log_warn(log, "USSD Submit: USR=%d is invalid, no session", key.USR);
                SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
//            dst->delUMRMapping(key.abonentAddr, umr);
//            dst->delUMRMapping(key.abonentAddr, 0);
        }
        else
        {
            smsc_log_warn(log, "USSD Submit: USSD_OP=%d is invalid", ussd_op);
            SubmitResp(cmd,smsc::system::Status::USSDDLGREFMISM);
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
    if(!SmppManager::Instance().makeLongCall(cmd, session))
    {
        SubmitResp(cmd, smsc::system::Status::SYSERR);
        session->closeCurrentOperation();
        sm.releaseSession(session);
        registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSERR);
    }
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
    session->closeCurrentOperation();
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  if (!bSibincoUSSD) sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, original_ussd_op);
  failed = putCommand(SUBMIT, src, dst, ri, cmd);

  session->getLongCallContext().runPostProcessActions();
  sm.releaseSession(session);
  smsc_log_debug(log, "Submit: processed");

  if(failed)
  {
    SmppCommand resp=SmppCommand::makeSubmitSmResp("0",cmd->get_dialogId(),failed);
    resp.setEntity(dst);
    resp->get_resp()->setOrgCmd(cmd);
    resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);    
    processSubmitResp(resp);
  }
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

  if(!cmd.hasSession())
  {
      smsc_log_debug(log, "SubmitResp: got");

      if(!cmd->get_resp()->hasOrgCmd()) // hasOrgCmd is true in the case of inplace call from processSubmit due to failed putCommand
      {
          int srcUid = 0;
          if(cmd->get_resp()->expiredResp)
          {
            srcUid=cmd->get_resp()->expiredUid;
          }else
          {
            try
            {
              srcUid = src->getUid();
            }
            catch (std::exception& exc)
            {
              smsc_log_warn(log, "SubmitResp: Src entity disconnected. sid='%s', seq='%d'",
                            src->getSystemId(), cmd->get_dialogId());
    //          registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
              return;
            }
          }
          if(!reg.Get(srcUid, cmd->get_dialogId(), orgCmd))
          {
            smsc_log_warn(log,"SubmitResp: Original submit for submit response not found. sid='%s',seq='%d'",
                          src ? src->getSystemId() : "NULL",cmd->get_dialogId());
            return;
          }
      }
      else
      {
        cmd->get_resp()->getOrgCmd(orgCmd);
      }

      if(!orgCmd->essentialSlicedResponse(cmd->get_resp()->get_status() || cmd->get_resp()->expiredResp))
        return;
      
      dst=orgCmd.getEntity();
      cmd.setDstEntity(dst);
      sms=orgCmd->get_sms();

      cmd->get_resp()->set_sms(sms);
      cmd->set_serviceId(orgCmd->get_serviceId());
      cmd->set_operationId(orgCmd->get_operationId());
      cmd->get_resp()->set_dir(dsdSc2Srv);
      cmd->set_dialogId(orgCmd->get_orgDialogId());

      sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
      sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);

      cmd->get_resp()->setOrgCmd(orgCmd);

      key.abonentAddr=sms->getDestinationAddress();
      int umr = sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
      ussd_op = sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ? sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;
      key.USR = (ussd_op < 0) ? umr : src->getUSR(key.abonentAddr, umr);

      if(!scag::sessions::SessionManager::Instance().getSession(key, session, cmd))
      {
        return;
      }

      smsc_log_debug(log,"SubmitResp: got session. key='%s:%d' %s",key.abonentAddr.toString().c_str(),key.USR, cmd->get_resp()->expiredResp ? "(expired)" : "");
  }
  else
  {
      smsc_log_debug(log, "SubmitResp: continued...");
      dst = cmd.getDstEntity();
      sms = cmd->get_resp()->get_sms();

      ussd_op = sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ? sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;

      session = cmd.getSession();
      smsc_log_debug(log, "SubmitResp: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
  }

  if(!session.Get())
  {
    smsc_log_warn(log,"SubmitResp: Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
    smsc_log_debug(log, "SubmitResp: RuleEngine processing...");
    scag::re::RuleEngine::Instance().process(cmd,*session, st);
    smsc_log_debug(log, "SubmitResp: RuleEngine processed");

    if(st.status == scag::re::STATUS_LONG_CALL)
    {
        smsc_log_debug(log,"SubmitResp: long call initiate");
        if(SmppManager::Instance().makeLongCall(cmd, session))
        {
          return;
        }
        rs = smsc::system::Status::SYSERR;
    }
    else if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "SubmitResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }
        rs = st.result;
    }
  }

  if (ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_RESPONSE ||
      ussd_op == smsc::smpp::UssdServiceOpValue::USSN_REQUEST)
  {
    src->delUSRMapping(key.abonentAddr, key.USR); // Empty mapping on USSD dialog end
  }

  if(cmd.hasSession())
  {
    cmd.setSession(SessionPtr(NULL));
  }

  if (!(cmd->get_resp()->get_messageId()))
  {
    smsc_log_warn(log, "SubmitResp: messageId is null");
    cmd->get_resp()->set_messageId("");
  }

  if(rs != -2)
  {
      cmd->get_resp()->set_status(rs);
      rs = scag::stat::events::smpp::RESP_GW_REJECTED;
  }
  else if(cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
  {
    rs = scag::stat::events::smpp::RESP_FAILED;
  }
  else if(cmd->get_resp()->expiredResp)
  {
    rs = scag::stat::events::smpp::RESP_EXPIRED;
  }
  else if(cmd->get_resp()->get_status())
  {
    rs = scag::stat::events::smpp::RESP_REJECTED;
  }
  else
  {
    rs = scag::stat::events::smpp::RESP_OK;
  }

  registerEvent(rs, src, dst, (char*)sms->getRouteId(), rs != scag::stat::events::smpp::RESP_OK ? cmd->get_resp()->get_status() : -1);

  try {
      dst->putCommand(cmd);
  } catch(std::exception& e)
  {
    smsc_log_warn(log,"SubmitResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
  }

  if(session.Get())
  {
    session->getLongCallContext().runPostProcessActions();
    scag::sessions::SessionManager::Instance().releaseSession(session);
  }
  cmd->get_resp()->set_sms(0);
  smsc_log_debug(log, "SubmitResp: processed");
}

void StateMachine::sendReceipt(SmppCommand& cmd)
{
  SmppEntity *dst = cmd.getDstEntity();
  try{
      if(dst->getBindType() == btNone)
      {
        smsc_log_info(log,"MSAG Receipt: sme not connected (%s)", dst->getSystemId());
      }
      else
      {
        int newSeq=dst->getNextSeq();
        if(!reg.Register(dst->getUid(), newSeq, cmd))
          throw Exception("MSAG Receipt: Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
        dst->putCommand(cmd);
      }
  } catch(std::exception& e)
  {
    smsc_log_info(log, "MSAG Receipt: Failed to putCommand into %s:%s", dst->getSystemId(), e.what());
  }
}

void StateMachine::processDelivery(SmppCommand& cmd)
{
    smsc_log_debug(log, "Delivery: got %s", cmd.hasSession() ? "continued..." : "");
    uint32_t rcnt = 0, failed = 0;
    SmppEntity *src = NULL; SmppEntity *dst = NULL;
    bool bSibincoUSSD = true; int original_ussd_op = -1;
    scag::sessions::SessionPtr session;
    scag::sessions::CSessionKey key;
    router::RouteInfo ri;
    buf::FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;
    scag::re::RuleStatus st;
    SMS& sms=*(cmd->get_sms());
    SmsCommand& smscmd=cmd->get_smsCommand();
    scag::sessions::SessionManager& sm = scag::sessions::SessionManager::Instance();
    cmd->set_orgDialogId(cmd->get_dialogId());

    if(cmd->flagSet(SmppCommandFlags::NOTIFICATION_RECEIPT))
    {
        sendReceipt(cmd);
        return;
    }

    int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
              sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;
    int ussd_op = sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
                  sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;
    if (ussd_op == 35) // Not Sibinco USSD dialog
    {
        smsc_log_debug(log, "Deliver: Not Sibinco USSD dialog for %s", 
                       sms.getOriginatingAddress().toString().c_str());
        sms.dropProperty(Tag::SMPP_USSD_SERVICE_OP);
        original_ussd_op = ussd_op; ussd_op = -1; umr = -1; bSibincoUSSD = false;
    }

    smscmd.dir = dsdSc2Srv;
    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src=cmd.getEntity();

    do
    {
      dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
      smsc_log_debug(log, "orig_route_id=%s, new_route_id=%s", routeId.c_str(), ri.routeId.c_str());
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

      smsc_log_info(log, "Delivery %s: %s UMR=%d, USSD_OP=%d. %s(%s)->%s, routeId=%s", cmd.hasSession() ? "continued..." : "", rcnt ? "(redirected)" : "", umr, ussd_op,
        sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str(), routeId.c_str());

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
                  if (umr > 0)
                  {
                      smsc_log_warn(log, "USSD Delivery: UMR=%d is set. "
                                         "Created new session instead. USR=%d", umr, key.USR);
                      umr = -1;
                  }
              }
          }
          else
          {
            session->getLongCallContext().continueExec = true;
          }
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

        if(cmd.hasSession())
        {
            session = cmd.getSession();
            smsc_log_debug(log, "Delivery(USSD): session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
        }
        else if (umr < 0)
        {
            smsc_log_warn(log, "USSD Delivery: UMR is not specified");
            DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM);
            registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
            return;
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::PSSR_INDICATION)
        { // New user USSD dialog
            smsc_log_debug(log, "USSD Delivery: Begin user dialog, UMR=%d", umr);
            session=sm.newSession(key);
            sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
            smsc_log_debug(log, "USSD Delivery: Creating mapping USR=%d, UMR=%d", key.USR, umr);
            if (src) src->setMapping(key.abonentAddr, umr, key.USR);
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSR_CONFIRM)
        { // Continue USSD dialog
            int tmpUSR = src->getUSR(key.abonentAddr, umr);
            if (tmpUSR == -1)
            {
                tmpUSR = src->getUSR(key.abonentAddr, 0);
                if(tmpUSR == -1) smsc_log_warn(log, "USSD Delivery: USR mapping not found");
                src->delUMRMapping(key.abonentAddr, 0);
                src->setMapping(key.abonentAddr, umr, tmpUSR);
            }
            key.USR = tmpUSR;
            smsc_log_debug(log, "USSD Delivery: Continue USR=%d", key.USR);
            sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
            if(!sm.getSession(key, session, cmd)) return;
            if (!session.Get())
            {
                smsc_log_warn(log, "USSD Delivery: USR=%d is invalid, no session", key.USR);
                DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
        }
        else if (ussd_op == smsc::smpp::UssdServiceOpValue::USSN_CONFIRM)
        { // End service USSD dialog
            int tmpUSR = src->getUSR(key.abonentAddr, umr);
            if (tmpUSR == -1)
            {
                tmpUSR = src->getUSR(key.abonentAddr, 0);
                if(tmpUSR == -1) smsc_log_warn(log, "USSD Delivery: USR mapping not found");
            }
            src->delUMRMapping(key.abonentAddr, umr);
            src->delUMRMapping(key.abonentAddr, 0); // For service initiated USSD dialog
            key.USR = tmpUSR;
            smsc_log_debug(log, "USSD Delivery: End service dialog, USR=%d", key.USR);
            if(!sm.getSession(key, session, cmd))
            {
              return;
            }
            if (!session.Get())
            {
                smsc_log_warn(log, "USSD Delivery: USR=%d is invalid, no session", key.USR);
                DeliveryResp(cmd,smsc::system::Status::USSDDLGREFMISM);
                registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::USSDDLGREFMISM);
                return;
            }
        }
        else
        {
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
      if(!SmppManager::Instance().makeLongCall(cmd, session))
      {
        DeliveryResp(cmd, smsc::system::Status::SYSERR);
        session->closeCurrentOperation();
        sm.releaseSession(session);
        registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSERR);
      }
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
    session->closeCurrentOperation();
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  if (!bSibincoUSSD) sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, original_ussd_op);
  failed = putCommand(DELIVERY, src, dst, ri, cmd);
  
  session->getLongCallContext().runPostProcessActions();
  sm.releaseSession(session);
  smsc_log_debug(log, "Delivery: processed");

  if(failed)
  {
    SmppCommand resp=SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),failed);
    resp.setEntity(dst);
    resp->get_resp()->setOrgCmd(cmd);
    resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);
    processDeliveryResp(resp);
  }
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

  if(!cmd.hasSession())
  {
      smsc_log_debug(log, "DeliveryResp: got");

      SmppCommand orgCmd;

      if(!cmd->get_resp()->hasOrgCmd())
      {
          int srcUid = 0;
          if(cmd->get_resp()->expiredResp)
          {
            srcUid=cmd->get_resp()->expiredUid;
          } else
          {
            try {
              srcUid = src->getUid();
            }
            catch (std::exception& exc)
            {
              smsc_log_warn(log, "DeliveryResp: Src entity disconnected. sid='%s', seq='%d'",
                            src->getSystemId(), cmd->get_dialogId());
        //          registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
              return;
            }
          }
          bool bGotFromRegistry = reg.Get(srcUid, cmd->get_dialogId(), orgCmd);
          if(bGotFromRegistry && orgCmd->flagSet(SmppCommandFlags::NOTIFICATION_RECEIPT))
          {
            smsc_log_debug(log, "MSAG Receipt: Got responce, expired (srcuid='%d', seq='%d')", srcUid, cmd->get_dialogId());
            return;
          }
          if(!bGotFromRegistry)
          {
            smsc_log_warn(log,"%s: Original delivery for delivery response not found. sid='%s',seq='%d'",
                  src ? "DeliveryResp" : "MSAG Receipt" , src ? src->getSystemId() : "NULL", cmd->get_dialogId());
            return;
          }
      }
      else
      {
        cmd->get_resp()->getOrgCmd(orgCmd);
      }

      if(!orgCmd->essentialSlicedResponse(cmd->get_resp()->get_status() || cmd->get_resp()->expiredResp))
        return;

      dst=orgCmd.getEntity();
      cmd.setDstEntity(dst);
      sms=orgCmd->get_sms();

      cmd->get_resp()->set_sms(sms);
      cmd->set_serviceId(orgCmd->get_serviceId());
      cmd->set_operationId(orgCmd->get_operationId());
      cmd->get_resp()->set_dir(dsdSrv2Sc);
      cmd->set_dialogId(orgCmd->get_orgDialogId());

      sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
      sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);

      cmd->get_resp()->setOrgCmd(orgCmd);

      key.abonentAddr = sms->getOriginatingAddress();
      key.USR = sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);

      if(!scag::sessions::SessionManager::Instance().getSession(key, session, cmd))
      {
        return;
      }

      smsc_log_debug(log,"DeliveryResp: got session. key='%s:%d' %s",key.abonentAddr.toString().c_str(),key.USR, cmd->get_resp()->expiredResp ? "(expired)" : "");
  }
  else
  {
    smsc_log_debug(log, "DeliveryResp: continued...");
    dst = cmd.getDstEntity();
    sms = cmd->get_resp()->get_sms();

    session = cmd.getSession();
    smsc_log_debug(log, "DeliveryResp: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
  }

  if(!session.Get())
  {
    smsc_log_warn(log,"DeliveryResp: Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::TRANSACTIONTIMEDOUT);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
//    smsc_log_debug(log, "sms:%x sm: %d mp: %d rsm: %d rmp: %d", sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE), sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD), sms->hasBinProperty(Tag::SMSC_RAW_SHORTMESSAGE), sms->hasBinProperty(Tag::SMSC_RAW_PAYLOAD));
    smsc_log_debug(log, "DeliveryResp: RuleEngine processing...");
    scag::re::RuleEngine::Instance().process(cmd,*session, st);
    smsc_log_debug(log, "DeliveryResp: RuleEngine processed.");

    if(st.status == scag::re::STATUS_LONG_CALL)
    {
  //          if (!reg.Register(srcUid, cmd->get_dialogId(), orgCmd))
  //            throw Exception("DeliveryResp: Register cmd for uid=%d, seq=%d failed", dst->getUid(), cmd->get_dialogId());

        smsc_log_debug(log,"DeliveryResp: long call initiate");
        if(SmppManager::Instance().makeLongCall(cmd, session))
        {
          return;
        }
        rs = smsc::system::Status::SYSERR;
    }
    else if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "DeliveryResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }

        rs = st.result;
    }
  }

  if(cmd.hasSession())
  {
    cmd.setSession(SessionPtr(NULL));
  }

  cmd->get_resp()->set_messageId("");

  if(rs != -2)
  {
      cmd->get_resp()->set_status(rs);
      rs = scag::stat::events::smpp::RESP_GW_REJECTED;
  }
  else if(cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
  {
    rs = scag::stat::events::smpp::RESP_FAILED;
  }
  else if(cmd->get_resp()->expiredResp)
  {
    rs = scag::stat::events::smpp::RESP_EXPIRED;
  }
  else if(cmd->get_resp()->get_status())
  {
    rs = scag::stat::events::smpp::RESP_REJECTED;
  }
  else
  {
    rs = scag::stat::events::smpp::RESP_OK;
  }

  registerEvent(rs, src, dst, (char*)sms->getRouteId(), rs != scag::stat::events::smpp::RESP_OK ? cmd->get_resp()->get_status() : -1);

  try{
    dst->putCommand(cmd);
  } catch(std::exception& e)
  {
    smsc_log_warn(log,"DeliveryResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
  }

  if(session.Get())
  {
    session->getLongCallContext().runPostProcessActions();
    scag::sessions::SessionManager::Instance().releaseSession(session);
  }
  cmd->get_resp()->set_sms(0);
  smsc_log_debug(log, "DeliveryResp: processed");
}

void StateMachine::SubmitResp(SmppCommand& cmd,int status)
{
  cmd.getEntity()->putCommand(
    SmppCommand::makeSubmitSmResp(
      "0",
      cmd->get_orgDialogId(),
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
      cmd->get_orgDialogId(),
      status
    )
  );
}

void StateMachine::DataResp(SmppCommand& cmd,int status)
{
  cmd.getEntity()->putCommand(
    SmppCommand::makeDataSmResp(
      "0",
      cmd->get_orgDialogId(),
      status
    )
  );
}

void StateMachine::processDataSm(SmppCommand& cmd)
{
    smsc_log_debug(log, "DataSm: got %s", cmd.hasSession() ? "continued..." : "");
    uint32_t rcnt = 0, failed = 0;
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

    cmd->set_orgDialogId(cmd->get_dialogId());


    if(sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
    {
        smsc_log_info(log,"DataSm: USSD dialog not allowed in DataSm %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
                        src->getSystemId(), sms.getDestinationAddress().toString().c_str());
        DataResp(cmd,smsc::system::Status::USSDDLGREFMISM);
        registerEvent(scag::stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::USSDDLGREFMISM);
        return;
    }

    do{
      dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
      smsc_log_debug(log, "orig_route_id=%s, new_route_id=%s", routeId.c_str(), ri.routeId.c_str());
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

      if(src->info.type==etService)
      {
        smscmd.dir = (dst->info.type==etService) ? dsdSrv2Srv : dsdSrv2Sc;
      }
      else
      {
        smscmd.dir = (dst->info.type==etService) ? dsdSc2Srv : dsdSc2Sc;
      }

      cmd->set_serviceId(ri.serviceId);

      int umr = sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) ?
                sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) : -1;

      smsc_log_info(log, "DataSm %s: %s UMR=%d. %s(%s)->%s, routeid=%s", cmd.hasSession() ? "continued..." : "", rcnt ? "(redirected)" : "", umr,
                sms.getOriginatingAddress().toString().c_str(), src->getSystemId(), sms.getDestinationAddress().toString().c_str(), routeId.c_str());

      key.abonentAddr = (src->info.type == etService) ? sms.getDestinationAddress() : sms.getOriginatingAddress();
      if(!session.Get())
      {
        if(cmd.hasSession())
        {
            session = cmd.getSession();
            smsc_log_debug(log, "DataSm: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
        }
        else if (umr < 0)
        {
            key.USR = 0;
            session=sm.newSession(key);
            sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
        }
        else
        {
            key.USR = umr;
            smsc_log_debug(log, "DataSm: Continue, UMR=%d", umr);
            if(!sm.getSession(key, session, cmd)) return;
            if (!session.Get())
            {
                session=sm.newSession(key);
                sms.setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE, key.USR);
                smsc_log_warn(log, "DataSm: Session for USR=%d not found, created new USR=%d, Address=%s", umr, key.USR, key.abonentAddr.toString().c_str());
            }
        }
      }
      else
      {
        session->getLongCallContext().continueExec = true;
      }
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
      if(!SmppManager::Instance().makeLongCall(cmd, session))
      {
        DataResp(cmd, smsc::system::Status::SYSERR);
        session->closeCurrentOperation();
        sm.releaseSession(session);
        registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSERR);
      }
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
    session->closeCurrentOperation();
    sm.releaseSession(session);
    registerEvent(scag::stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
    return;
  }

  failed = putCommand(DATASM, src, dst, ri, cmd);

  session->getLongCallContext().runPostProcessActions();
  sm.releaseSession(session);
  smsc_log_debug(log, "DataSm: processed");

  if(failed)
  {
    SmppCommand resp=SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),failed);
    resp.setEntity(dst);
    resp->get_resp()->setOrgCmd(cmd);
    resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);    
    processDataSmResp(resp);
  }
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

  if(!cmd.hasSession())
  {
      smsc_log_debug(log, "DataSmResp: got");

      SmppCommand orgCmd;
      if(!cmd->get_resp()->hasOrgCmd())
      {
          int srcUid = 0;
          if(cmd->get_resp()->expiredResp)
          {
            srcUid=cmd->get_resp()->expiredUid;
          }else
          {
            try {
              srcUid = src->getUid();
            }
            catch (std::exception& exc)
            {
              smsc_log_warn(log, "DataSmResp: Src entity disconnected. sid='%s', seq='%d'",
                            src ? src->getSystemId() : "NULL", cmd->get_dialogId());
    //          registerEvent(scag::stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
              return;
            }
          }
          if(!reg.Get(srcUid, cmd->get_dialogId(), orgCmd))
          {
            smsc_log_warn(log,"DataSmResp: Original datasm for datasm response not found. sid='%s',seq='%d'",src->getSystemId(),cmd->get_dialogId());
            return;
          }
      }
      else
      {
        cmd->get_resp()->getOrgCmd(orgCmd);
      }

      if(!orgCmd->essentialSlicedResponse(cmd->get_resp()->get_status() || cmd->get_resp()->expiredResp))
        return;

      SmsCommand& smscmd=orgCmd->get_smsCommand();
      sms=orgCmd->get_sms();
      cmd->get_resp()->set_sms(sms);

      if (smscmd.dir == dsdSrv2Sc)
      {
        cmd->get_resp()->set_dir(dsdSc2Srv);
      }
      else if (smscmd.dir == dsdSc2Srv)
      {
        cmd->get_resp()->set_dir(dsdSrv2Sc);
      }
      else
      {
        cmd->get_resp()->set_dir(smscmd.dir);
      }

      cmd->set_serviceId(orgCmd->get_serviceId());
      cmd->set_operationId(orgCmd->get_operationId());
      sms->setOriginatingAddress(smscmd.orgSrc);
      sms->setDestinationAddress(smscmd.orgDst);

      dst=orgCmd.getEntity();
      cmd.setDstEntity(dst);
      cmd->set_dialogId(orgCmd->get_orgDialogId());

      cmd->get_resp()->setOrgCmd(orgCmd);

      key.abonentAddr = (cmd->get_resp()->get_dir() == dsdSc2Srv || cmd->get_resp()->get_dir() == dsdSrv2Srv) ?
             sms->getDestinationAddress() : sms->getOriginatingAddress();
      key.USR=sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);

      if(!scag::sessions::SessionManager::Instance().getSession(key, session, cmd))
      {
        return;
      }

      smsc_log_debug(log,"DataSmResp: got session. key='%s:%d' %s",key.abonentAddr.toString().c_str(),key.USR, cmd->get_resp()->expiredResp ? "(expired)" : "");
  }
  else
  {
    smsc_log_debug(log, "DataSmResp: continued...");
    dst = cmd.getDstEntity();
    sms = cmd->get_resp()->get_sms();

    session = cmd.getSession();
    smsc_log_debug(log, "DataSmResp: session got from command USR=%d Address=%s", session->getSessionKey().USR, session->getSessionKey().abonentAddr.toString().c_str());
  }

  if(!session.Get())
  {
    smsc_log_warn(log,"Session not found. key='%s:%d'",key.abonentAddr.toString().c_str(),key.USR);
    cmd->get_resp()->set_status(smsc::system::Status::TRANSACTIONTIMEDOUT);
    rs = smsc::system::Status::TRANSACTIONTIMEDOUT;
  }
  else
  {
    smsc_log_debug(log, "DataSmResp: RuleEngine processing...");
    scag::re::RuleEngine::Instance().process(cmd,*session, st);
    smsc_log_debug(log, "DataSmResp: RuleEngine processed.");

    if(st.status == scag::re::STATUS_LONG_CALL)
    {
        smsc_log_debug(log,"DataSmResp: long call initiate");
        if(SmppManager::Instance().makeLongCall(cmd, session))
        {
          return;
        }
        rs = smsc::system::Status::SYSERR;
    }
    else if(st.status != scag::re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log, "DataSmResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }

        rs = st.result;
    }
  }

  if(cmd.hasSession())
  {
    cmd.setSession(SessionPtr(NULL));
  }

  if (!(cmd->get_resp()->get_messageId()))
  {
    cmd->get_resp()->set_messageId("");
  }

  if(rs != -2)
  {
      cmd->get_resp()->set_status(rs);
      rs = scag::stat::events::smpp::RESP_GW_REJECTED;
  }
  else if(cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
  {
    rs = scag::stat::events::smpp::RESP_FAILED;
  }
  else if(cmd->get_resp()->expiredResp)
  {
    rs = scag::stat::events::smpp::RESP_EXPIRED;
  }
  else if(cmd->get_resp()->get_status())
  {
    rs = scag::stat::events::smpp::RESP_REJECTED;
  }
  else
  {
    rs = scag::stat::events::smpp::RESP_OK;
  }

  registerEvent(rs, src, dst, (char*)sms->getRouteId(), rs != scag::stat::events::smpp::RESP_OK ? cmd->get_resp()->get_status() : -1);


  try{
    dst->putCommand(cmd);
  } catch(std::exception& e)
  {
    smsc_log_warn(log,"DataSmResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
  }

  if(session.Get())
  {
    session->getLongCallContext().runPostProcessActions();
    scag::sessions::SessionManager::Instance().releaseSession(session);
  }

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
