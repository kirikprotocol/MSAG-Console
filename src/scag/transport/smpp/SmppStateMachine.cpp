#include "SmppStateMachine.h"
#include "core/buffers/XHash.hpp"
#include "scag/re/RuleEngine.h"
#include "scag/sessions/SessionManager.h"

namespace scag{
namespace transport{
namespace smpp{

namespace buf=smsc::core::buffers;

struct StateMachine::ResponseRegistry{
  smsc::logger::Logger* log;
  ResponseRegistry()
  {
    log=0;
  }
  struct RegKey{
    int uid;
    int seq;
    bool operator==(const RegKey& key)
    {
      return uid==key.uid && seq==key.seq;
    }
  };
  struct RegValue{
    SmppCommand cmd;
    int dlgId;
  };
  struct HashFunc{
    static unsigned int CalcHash(const RegKey& key)
    {
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
    val.cmd=cmd;
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
    reg.Delete(key);
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
        smsc_log_debug(log,"Exec: processing command %d(%x) from %s",cmd->get_commandId(),cmd->get_commandId(),cmd.getEntity()->getSystemId());
        switch(cmd->get_commandId())
        {
          case SUBMIT:processSubmit(cmd);break;
          case SUBMIT_RESP:processSubmitResp(cmd);break;
          case DELIVERY:processDelivery(cmd);break;
          case DELIVERY_RESP:processDeliveryResp(cmd);break;
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

void StateMachine::processSubmit(SmppCommand& cmd)
{
  router::RouteInfo ri;
  SMS& sms=*cmd->get_sms();
  SmppEntity *src=cmd.getEntity();
  SmppEntity *dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
  if(!dst)
  {
    smsc_log_info(log,"Submit: noroute %s(%s)->%s",
    sms.getOriginatingAddress().toString().c_str(),
    src->getSystemId(),
    sms.getDestinationAddress().toString().c_str());
    SubmitResp(cmd,smsc::system::Status::NOROUTE);
    return;
  }

  sms.setIntProperty(Tag::SMSC_PROVIDERID,ri.providerId);
  sms.setIntProperty(Tag::SMSC_CATEGORYID,ri.categoryId);
  sms.setRouteId(ri.routeId);
  sms.setSourceSmeId(src->getSystemId());
  sms.setDestinationSmeId(dst->getSystemId());

  cmd->set_ruleId(ri.ruleId);


  scag::sessions::CSessionKey key;
  key.USR=sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE);
  key.abonentAddr=sms.getDestinationAddress();
  scag::sessions::Session* session=scag::sessions::SessionManager::Instance().newSession(key);

  scag::re::RuleStatus st=scag::re::RuleEngine::Instance().process(cmd,*session);

  if(!st.status)
  {
    smsc_log_info(log,"Submit: RuleEngine returned result=%d",st.result);
    SubmitResp(cmd,
      st.temporal?smsc::system::Status::RX_T_APPN:
                  smsc::system::Status::RX_P_APPN
    );
    scag::sessions::SessionManager::Instance().releaseSession(session);
    return;
  }


  try{
    int newSeq=dst->getNextSeq();
    reg.Register(dst->getUid(),newSeq,cmd);
    dst->putCommand(cmd);
  }catch(std::exception& e)
  {
    smsc_log_info(log,"Submit:Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
  }
  scag::sessions::SessionManager::Instance().releaseSession(session);
}

void StateMachine::processSubmitResp(SmppCommand& cmd)
{
  SmppCommand orgCmd;
  SmppEntity* src=cmd.getEntity();
  if(!reg.Get(src->getUid(),cmd->get_dialogId(),orgCmd))
  {
    smsc_log_warn(log,"Original submit for submit response not found. sid='%s',seq='%d'",src->getSystemId(),cmd->get_dialogId());
    return;
  }

  SmppEntity* dst=orgCmd.getEntity();

  cmd->set_dialogId(orgCmd->get_dialogId());

  try{
    dst->putCommand(cmd);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"SubmitResp:Failed to put command into %s:%s",dst->getSystemId(),e.what());
  }
}

void StateMachine::processDelivery(SmppCommand& cmd)
{
  router::RouteInfo ri;
  SMS& sms=*cmd->get_sms();
  SmppEntity *src=cmd.getEntity();
  SmppEntity *dst=routeMan->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
  if(!dst)
  {
    smsc_log_info(log,"Delivery: no route %s(%s)->%s",
      sms.getOriginatingAddress().toString().c_str(),
      src->getSystemId(),
      sms.getDestinationAddress().toString().c_str());
    DeliveryResp(cmd,smsc::system::Status::NOROUTE);
    return;
  }

  sms.setIntProperty(Tag::SMSC_PROVIDERID,ri.providerId);
  sms.setIntProperty(Tag::SMSC_CATEGORYID,ri.categoryId);
  sms.setRouteId(ri.routeId);
  sms.setSourceSmeId(src->getSystemId());
  sms.setDestinationSmeId(dst->getSystemId());

  cmd->set_ruleId(ri.ruleId);


  try{
    int newSeq=dst->getNextSeq();
    reg.Register(dst->getUid(),newSeq,cmd);
    dst->putCommand(cmd);
  }catch(std::exception& e)
  {
    smsc_log_info(log,"Delivery:Failed to putCommand into %s:%s",dst->getSystemId(),e.what());
  }
}

void StateMachine::processDeliveryResp(SmppCommand& cmd)
{
  SmppCommand orgCmd;
  SmppEntity* src=cmd.getEntity();
  if(!reg.Get(src->getUid(),cmd->get_dialogId(),orgCmd))
  {
    smsc_log_warn(log,"Original delivery for delivery response not found. sid='%s',seq='%d'",src->getSystemId(),cmd->get_dialogId());
    return;
  }

  cmd->get_resp()->set_messageId("");

  SmppEntity* dst=orgCmd.getEntity();

  try{
    dst->putCommand(cmd);
  }catch(std::exception& e)
  {
    smsc_log_warn(log,"DeliveryResp:Failed to put command into %s:%s",dst->getSystemId(),e.what());
  }
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


}//smpp
}//transport
}//scag
