#include <cassert>

#include "SmppManager2.h"
#include "SmppStateMachine2.h"
#include "SmppUtil.h"
#include "SmsSplit.h"
#include "core/buffers/XHash.hpp"
#include "scag/re/base/RuleEngine2.h"
#include "scag/sessions/base/SessionManager2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/stat/base/Statistics2.h"
#include "system/status.h"
#include "scag/transport/smpp/router/route_types.h"
#include "scag/config/route/RouteStructures.h"
#include "SmppOperationMaker.h"

namespace scag2 {
namespace transport {
namespace smpp {

using namespace lcm;
using namespace sessions;
using namespace stat;
using namespace scag::transport::smpp;
using namespace smsc::core::buffers;

std::vector<int> StateMachine::allowedUnknownOptionals;

const uint32_t MAX_REDIRECT_CNT = 10;

struct StateMachine::ResponseRegistry
{
    ResponseRegistry() : log(0), timeout(60) {}

    struct RegKey {
        RegKey(int _uid=-1, int _seq=-1):uid(_uid), seq(_seq) {}
        bool operator==(const RegKey& key)
        {
            return uid==key.uid && seq==key.seq;
        }
    public:
        int uid, seq;
    };

    struct ListValue {
        RegKey key;
        time_t insTime;
        // bool expired;
    };

    typedef std::list<ListValue> TimeOutList;

    struct RegValue {
        SmppCommand* cmd;  // owned
        TimeOutList::iterator it;
    };

    struct HashFunc{
        static unsigned int CalcHash(const RegKey& key) {
            return key.uid<<9+key.seq;
        }
    };


    bool Register( int uid, int seq, SmppCommand* cmd, bool limit = true)
    {
        if (!cmd) return false;
        CommandId cmdid( CommandId(cmd->getCommandId()) );
        assert( cmdid == SUBMIT || cmdid == DELIVERY || cmdid == DATASM );

        RegKey key(uid, seq);
        sync::MutexGuard mg(mtx);
        if (!log) log=smsc::logger::Logger::getInstance("respreg");
        if (reg.Exists(key))
        { // key already registered
            smsc_log_warn(log, "register %d/%d failed", uid, seq);
            return false;
        }
        if(cmd->getDstEntity()->info.outQueueLimit > 0)
        {
            int* p=outCnt.GetPtr(cmd->getDstEntity()->info.systemId.c_str());
            if(limit && p && *p>cmd->getDstEntity()->info.outQueueLimit)
            {
                smsc_log_warn(log,"command registration for '%s' denied: outQueueLimit",
                              cmd->getDstEntity()->info.systemId.c_str());
                return false;
            }
            if(p)
            {
                (*p)++;
            }else
            {
                outCnt.Insert(cmd->getDstEntity()->info.systemId.c_str(),1);
            }
        }
        RegValue val;
        ListValue lv;
        lv.key = key;
        lv.insTime = time(NULL);
        // lv.expired = false;
        toList.push_back(lv);
        val.cmd = cmd->clone().release();
        val.it = toList.end();
        val.it--;
        // we will use dialog id in resp
        cmd->set_dialogId( seq );
        reg.Insert(key,val);
        smsc_log_debug(log, "register uid=%d, seq=%d, name=%s", uid, seq, cmd->getDstEntity()->info.systemId.c_str());
        return true;
    }


    std::auto_ptr<SmppCommand> Get( int uid, int seq )
    {
        sync::MutexGuard mg(mtx);
        if (!log) log = smsc::logger::Logger::getInstance("respreg");
        RegKey key(uid, seq);
        RegValue* ptr=reg.GetPtr(key);
        smsc_log_debug(log, "get uid=%d seq=%d - %s", uid, seq, (ptr) ? "ok":"not found");
        std::auto_ptr<SmppCommand> cmd;
        if (ptr) {
            cmd.reset(ptr->cmd);
            //cmd->set_dialogId(ptr->dlgId);
            toList.erase(ptr->it);
            reg.Delete(key);
            if(cmd->getDstEntity()->info.outQueueLimit > 0) {
                outCnt.Get(cmd->getDstEntity()->info.systemId.c_str())--;
            }
        }
        return cmd;
    }


    std::auto_ptr<SmppCommand> getExpiredCmd( int &uid )
    {
        sync::MutexGuard mg(mtx);
        std::auto_ptr<SmppCommand> cmd;
        do { // fake loop

            if (toList.empty()) break;

            time_t now = time(NULL);
            if ((now - toList.front().insTime) < timeout) break;

            /*
            if(toList.front().expired)
            {
                RegValue* ptr=reg.GetPtr(toList.begin()->key);
                if(ptr && ptr->cmd->getDstEntity()->info.outQueueLimit > 0)
                {
                    outCnt.Get(ptr->cmd->getDstEntity()->info.systemId.c_str())--;
                }
                reg.Delete(toList.begin()->key);
                toList.erase(toList.begin());
                return false;
            }
             */
            RegKey key = toList.front().key;
            RegValue* ptr = reg.GetPtr(key);
            if (!ptr)
            {
                // toList cleanup (find listValue by key & delete it)?
                toList.erase(toList.begin());
                break;
            }

            cmd.reset(ptr->cmd);
            uid = key.uid;
            toList.erase(toList.begin());
            reg.Delete(key);
            cmd->setFlag(SmppCommandFlags::EXPIRED_COMMAND);

            if ( cmd->getDstEntity()->info.outQueueLimit > 0 ) {
                outCnt.Get(cmd->getDstEntity()->info.systemId.c_str())--;
            }

        } while ( false );
        return cmd;
    }

private:
    smsc::logger::Logger* log;
    int timeout;
    TimeOutList toList;
    XHash<RegKey, RegValue, HashFunc> reg;
    Hash<int> outCnt;
    sync::Mutex mtx;
};


StateMachine::ResponseRegistry StateMachine::reg_;

sync::Mutex StateMachine::expMtx_;
bool StateMachine::expProc_ = false;


int StateMachine::Execute()
{
  SmppCommand* pcmd;
  while(!isStopping)
  {
    try{
      while(queue_->getCommand(pcmd))
      {
          if ( !pcmd ) continue;
          std::auto_ptr<SmppCommand> aucmd(pcmd);
          SmppCommand* cmd = pcmd;

          if ( cmd->getCommandId() != PROCESSEXPIREDRESP ) {
              smsc_log_debug(log_,"Exec: processing command %p %d(%x) from %s", pcmd,
                             cmd->getCommandId(), cmd->getCommandId(), cmd->getEntity()?cmd->getEntity()->getSystemId():"" );
          }

          switch(cmd->getCommandId())
          {
          case SUBMIT:              processSubmit(aucmd);             break;
          case SUBMIT_RESP:         processSubmitResp(aucmd);         break;
          case DELIVERY:            processDelivery(aucmd);           break;
          case DELIVERY_RESP:       processDeliveryResp(aucmd);       break;
          case DATASM:              processDataSm(aucmd);             break;
          case DATASM_RESP:         processDataSmResp(aucmd);         break;
          case PROCESSEXPIREDRESP:  processExpiredResps();            break;
          case ALERT_NOTIFICATION:  processAlertNotification(aucmd);  break;
          default:
              smsc_log_warn(log_,"Unprocessed command %p id %d", pcmd, cmd->getCommandId());
              break;
          }
      }
    }catch(std::exception& e)
    {
      smsc_log_error(log_,"Exception in state machine:%s",e.what());
    }
  } // while
  return 0;
}


void StateMachine::registerEvent(int event, SmppEntity* src, SmppEntity* dst, const char* rid, int errCode)
{
    char* src_id;
    char* dst_id = NULL;
    bool srcType, dstType = false;

    smsc_log_debug(log_, "SmppStateMachine Event:%d", event);

    src_id = (char*)src->info.systemId;
    srcType = src->info.type == etSmsc;

    if(dst)
    {
        dst_id = (char*)dst->info.systemId;
        dstType = dst->info.type == etSmsc;
    }

    Statistics::Instance().registerEvent(SmppStatEvent(src_id, srcType, dst_id, dstType, rid, event, errCode));
}

    
uint32_t StateMachine::putCommand(CommandId cmdType, SmppEntity* src, SmppEntity* dst, router::RouteInfo& ri, std::auto_ptr<SmppCommand>& aucmd)
{
    if ( ! aucmd.get() ) return smsc::system::Status::SYSFAILURE;
    SmppCommand* cmd = aucmd.get();
    const char *cmdName;
    switch ( cmdType ) {
    case SUBMIT :
        cmdName = "SubmitSm";
        break;
    case DELIVERY:
        cmdName = "DeliverySm";
        break;
    case DATASM :
        cmdName = "DataSm";
        break;
    default:
        cmdName = 0;
    }
    uint32_t failed = 0;
    SMS& sms = *cmd->get_sms();

  /*
   FIXME: release session externally
  if(cmd.hasSession())
    cmd.setSession(SessionPtr(NULL)); // cmd can lay in out queue quite long time, so let's free session right now.
   */

    try{
        if ( cmdName == 0 ) {
            smsc_log_info(log_,"wrong command type in putCommand: %d", int(cmdType) );
            failed = smsc::system::Status::INVCMDID;
        } else if (dst->getBindType() == btNone)
            {
                smsc_log_info(log_,"%s: sme not connected %s(%s)->%s(%s)", cmdName, sms.getOriginatingAddress().toString().c_str(), src->getSystemId(),
                              sms.getDestinationAddress().toString().c_str(), dst->getSystemId());
                failed = smsc::system::Status::SMENOTCONNECTED;
            }
        else
        {
            stripUnknownSmppOptionals( sms,allowedUnknownOptionals );

            uint32_t cnt;
            if( ri.slicing != router::SlicingType::NONE && 
                !isSliced(sms) && (cnt = getPartsCount(sms)) > 1)
            {
                smsc_log_debug(log_, "%s: slicing message, type=%d, parts=%d, resppolicy=%d", cmdName, ri.slicing, cnt, ri.slicingRespPolicy);
                cmd->get_smsCommand().setSlicingParams(ri.slicingRespPolicy, cnt);
                uint32_t seq = 0, refNum = dst->getNextSlicingSeq();

                SMS partSms;
                uint8_t udhType = ri.slicing > router::SlicingType::SAR ?
                    ri.slicing - router::SlicingType::SAR : 0;
                while( (seq = getNextSmsPart(sms, partSms, refNum, seq, udhType, cmdType == DATASM)) )
                {
                    int newSeq=dst->getNextSeq();
                    std::auto_ptr<SmppCommand> partCmd( SmppCommand::makeCommandSm(cmdType, partSms, newSeq) );
                    if (!reg_.Register(dst->getUid(), newSeq, aucmd.get(), seq == 1))
                        throw Exception("%s: Register cmd for uid=%d, seq=%d failed", cmdName, dst->getUid(), newSeq);
                    dst->putCommand(partCmd);
                }
            }
            else
            {
                int newSeq=dst->getNextSeq();
                if (!reg_.Register(dst->getUid(), newSeq, aucmd.get()))
                    throw Exception("%s: Register cmd for uid=%d, seq=%d failed", cmdName, dst->getUid(), newSeq);
                dst->putCommand(aucmd);
            }
            registerEvent( stat::events::smpp::ACCEPTED, src, dst, (char*)ri.routeId, -1);
        }
    } catch (std::exception& e) {
        failed = smsc::system::Status::SYSFAILURE;
        smsc_log_info(log_,"%s: Failed to putCommand into %s:%s", cmdName, dst->getSystemId(),e.what());
    }
    return failed;
}


void StateMachine::processSubmit( std::auto_ptr<SmppCommand> aucmd)
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    smsc_log_debug(log_, "Submit: got cmd=%p %s", cmd, cmd->getSession() ? "continued..." : "");
    uint32_t rcnt = 0, failed = 0;
    SmppEntity *src = NULL;
    SmppEntity *dst = NULL;
    ActiveSession session; // to unlock session automatically
    router::RouteInfo ri;
    FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;
    re::RuleStatus st;
    SMS& sms = *cmd->get_sms();
    SmsCommand& smscmd = cmd->get_smsCommand();
    SessionManager& sm = SessionManager::Instance();

    const int ussd_op = getUSSDOp("Submit", sms, &smscmd );

    smscmd.dir = dsdSrv2Sc;
    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src = cmd->getEntity();
    smscmd.set_orgDialogId(cmd->get_dialogId());

    do { // rerouting loop

        dst=routeMan_->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
        smsc_log_debug(log_, "orig_route_id=%s, new_route_id=%s", routeId.c_str(), ri.routeId.c_str());
        if (!dst || routeId == ri.routeId)
        {
            smsc_log_info(log_,"Submit: %s %s(%s)->%s", !dst ? "no route" : "redirection to the same route",
                          sms.getOriginatingAddress().toString().c_str(), src->getSystemId(),
                          sms.getDestinationAddress().toString().c_str());
            SubmitResp( aucmd, smsc::system::Status::NOROUTE );
            registerEvent( stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
            if (session.get()) session->closeCurrentOperation();
            return;
        }
        routeId = ri.routeId;
        cmd->setDstEntity(dst);
        sms.setRouteId(ri.routeId);
        sms.setSourceSmeId(src->getSystemId());
        sms.setDestinationSmeId(dst->getSystemId());
        cmd->setServiceId(ri.serviceId);

        smsc_log_info( log_, "Submit %s: %s, USSD_OP=%d. %s(%s)->%s, routeId=%s",
                       cmd->getOperationId() != SCAGCommand::invalidOpId() ? "continued..." : "",
                       rcnt ? "(redirected)" : "", ussd_op,
                       sms.getOriginatingAddress().toString().c_str(),
                       src->getSystemId(),
                       sms.getDestinationAddress().toString().c_str(),
                       routeId.c_str() );

        if ( ! session.get() ) {

            SessionKey key( sms.getDestinationAddress() );
            session = sm.getSession( key, aucmd );
            if ( ! session.get() ) return; // locked

        }

        if ( ussd_op >= 0 && rcnt > 0 ) {
            smsc_log_warn(log_, "USSD Submit: Rerouting for USSD dialog not allowed");
            SubmitResp(aucmd, smsc::system::Status::NOROUTE);
            registerEvent( stat::events::smpp::REJECTED, src, dst, NULL, smsc::system::Status::NOROUTE);
            session->closeCurrentOperation();
            return;
        }

        SmppOperationMaker opmaker( *cmd, *session.get() );
        opmaker.setupOperation( st );
        if ( st.status != re::STATUS_OK ) {
            smsc_log_warn( log_, "Submit op fail: %s, res=%d", opmaker.what(), st.result );
            SubmitResp( aucmd, st.result );
            registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result );
            return;
        }

        smsc_log_debug(log_, "Submit: RuleEngine processing...");
        re::RuleEngine::Instance().process( *cmd, *session.get(), st);
        smsc_log_debug(log_, "Submit: RuleEngine processed: st.status=%d st.result=%d", st.status, st.result );

        opmaker.postProcess( st );

    } while ( st.status == re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

    if (rcnt >= MAX_REDIRECT_CNT)
    {
        smsc_log_info(log_,"Submit: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
                      src->getSystemId(), sms.getDestinationAddress().toString().c_str());
        SubmitResp(aucmd,smsc::system::Status::NOROUTE);
        registerEvent( stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        session->closeCurrentOperation();
        return;
    }

    if(st.status == re::STATUS_LONG_CALL)
    {
        smsc_log_info(log_,"Submit: long call initiate");
        if(!SmppManager::Instance().makeLongCall(aucmd, session))
        {
            SubmitResp(aucmd, smsc::system::Status::SYSERR);
            session->closeCurrentOperation();
            registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSERR);
        }
        return;
    }

    if(st.status != re::STATUS_OK)
    {
        smsc_log_info(log_,"Submit: RuleEngine returned result=%d",st.result);
        if(!st.result)
        {
            smsc_log_warn(log_, "Submit: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }
        SubmitResp(aucmd, st.result);
        session->closeCurrentOperation();
        registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
        return;
    }

    if (smscmd.original_ussd_op != -1) // Not Sibinco USSD
        sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, smscmd.original_ussd_op);
    failed = putCommand(SUBMIT, src, dst, ri, aucmd);

    session->getLongCallContext().runPostProcessActions();
    smsc_log_debug(log_, "Submit: processed");

    if(failed)
    {
        std::auto_ptr<SmppCommand> resp(SmppCommand::makeSubmitSmResp("0",cmd->get_dialogId(),failed));
        resp->setEntity(dst);
        resp->get_resp()->setOrgCmd( aucmd.release() );
        resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);
        processSubmitResp(resp,session);
    }
}


void StateMachine::processSubmitResp(std::auto_ptr<SmppCommand> aucmd, ActiveSession session )
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    int rs = -2;
    re::RuleStatus st;

    SmppEntity *dst, *src;
    SMS *sms;

    src = cmd->getEntity();

    do { // fake loop

        if ( cmd->getOperationId() == SCAGCommand::invalidOpId() ) {

            smsc_log_debug(log_, "SubmitResp: got cmd=%p", cmd );
            SmppCommand* orgCmd;

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
                        smsc_log_warn(log_, "SubmitResp: Src entity disconnected. sid='%s', seq='%d'",
                                      src->getSystemId(), cmd->get_dialogId());
                        //          registerEvent( stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
                        return;
                    }
                }

                std::auto_ptr<SmppCommand> auOrgCmd(reg_.Get(srcUid, cmd->get_dialogId()));
                orgCmd = auOrgCmd.get();
                if(! orgCmd )
                {
                    smsc_log_warn(log_,"SubmitResp: Original submit for submit response not found. sid='%s',seq='%d'",
                                  src ? src->getSystemId() : "NULL",cmd->get_dialogId());
                    return;
                }
                cmd->get_resp()->setOrgCmd(auOrgCmd.release());
            }
            else
            {
                orgCmd = cmd->get_resp()->getOrgCmd();
            }

            if(!orgCmd->get_smsCommand().essentialSlicedResponse(cmd->get_status() || cmd->get_resp()->expiredResp))
                return; // not all chunks are responsed

            dst = orgCmd->getEntity();
            cmd->setDstEntity(dst);
            sms = orgCmd->get_sms();

            // cmd->get_resp()->set_sms(sms);
            cmd->setServiceId( orgCmd->getServiceId() );

            // will be done in opmaker
            // cmd->setOperationId( orgCmd->getOperationId() );

            cmd->get_resp()->set_dir(dsdSc2Srv);
            cmd->set_dialogId(orgCmd->get_smsCommand().get_orgDialogId());

            sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
            sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);

        } else {

            smsc_log_debug(log_, "SubmitResp: continued... cmd=%p", cmd );
            dst = cmd->getDstEntity();
            sms = cmd->get_resp()->getOrgCmd()->get_sms();

        }

        getUSSDOp( "SubmitResp", *sms, 0 );

        // if session is already locked
        if ( session.get() ) {
            session.moveLock(cmd);
            smsc_log_debug(log_,"SubmitResp: got pre-locked session=%p", session.get() );
            break;
        }

        SessionKey key( sms->getDestinationAddress() );
        session = SessionManager::Instance().getSession(key, aucmd);
        if ( ! session.get() )
            return;
        
        smsc_log_debug(log_,"SubmitResp: got session=%p key='%s' %s",
                       session.get(), key.toString().c_str(), cmd->get_resp()->expiredResp ? "(expired)" : "");

    } while ( false ); // fake loop


    // create operation
    SmppOperationMaker opmaker( *cmd, *session.get() );
    opmaker.setupOperation( st );
    if ( st.status != re::STATUS_OK ) {
        smsc_log_warn( log_, "SubmitResp op fail: %s, res=%d", opmaker.what(), st.result );
        registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)sms->getRouteId(), st.result );
    }

    smsc_log_debug(log_, "SubmitResp: RuleEngine processing...");
    re::RuleEngine::Instance().process( *cmd, *session.get(), st );
    smsc_log_debug(log_, "SubmitResp: RuleEngine processed: st.status=%d st.result=%d", st.status, st.result );

    opmaker.postProcess( st );

    if (st.status == re::STATUS_LONG_CALL)
    {
        smsc_log_debug(log_,"SubmitResp: long call initiate");
        if(SmppManager::Instance().makeLongCall(aucmd, session))
        {
            return;
        }
        rs = smsc::system::Status::SYSERR;
    }
    else if (st.status != re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log_, "SubmitResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }
        rs = st.result;
    }

    if (!(cmd->get_resp()->get_messageId())) {
        smsc_log_warn(log_, "SubmitResp: messageId is null");
        cmd->get_resp()->set_messageId("");
    }

    if(rs != -2)
    {
        cmd->set_status(rs);
        rs = stat::events::smpp::RESP_GW_REJECTED;
    }
    else if(cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
    {
        rs = stat::events::smpp::RESP_FAILED;
    }
    else if(cmd->get_resp()->expiredResp)
    {
        rs = stat::events::smpp::RESP_EXPIRED;
    }
    else if(cmd->get_status())
    {
        rs = stat::events::smpp::RESP_REJECTED;
    }
    else
    {
        rs = stat::events::smpp::RESP_OK;
    }

    registerEvent(rs, src, dst, (char*)sms->getRouteId(), rs != stat::events::smpp::RESP_OK ? cmd->get_status() : -1);

    try {
        dst->putCommand(aucmd);
    } catch(std::exception& e) {
        smsc_log_warn(log_,"SubmitResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
    }

    if(session.get())
    {
        session->getLongCallContext().runPostProcessActions();
    }
    smsc_log_debug(log_, "SubmitResp: processed");
}


void StateMachine::processDelivery(std::auto_ptr<SmppCommand> aucmd)
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    smsc_log_debug(log_, "Delivery: got cmd=%p %s", cmd, cmd->getSession() ? "continued..." : "");
    uint32_t rcnt = 0, failed = 0;
    SmppEntity *src = NULL;
    SmppEntity *dst = NULL;
    ActiveSession session;
    router::RouteInfo ri;
    FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;
    re::RuleStatus st;
    SMS& sms = *(cmd->get_sms());
    SmsCommand& smscmd = cmd->get_smsCommand();
    SessionManager& sm = SessionManager::Instance();
    smscmd.set_orgDialogId(cmd->get_dialogId());

    if (cmd->flagSet(SmppCommandFlags::NOTIFICATION_RECEIPT))
    {
        sendReceipt(aucmd);
        return;
    }

    const int ussd_op = getUSSDOp("Delivery", sms, &smscmd );

    smscmd.dir = dsdSc2Srv;
    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src = cmd->getEntity();

    do { // rerouting loop

        dst=routeMan_->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
        smsc_log_debug(log_, "orig_route_id=%s, new_route_id=%s", routeId.c_str(), ri.routeId.c_str());
        if (!dst || routeId == ri.routeId)
        {
            smsc_log_info(log_,"Delivery: %s %s(%s)->%s", !dst ? "no route" : "redirection to the same route",
                          sms.getOriginatingAddress().toString().c_str(),
                          src->getSystemId(),
                          sms.getDestinationAddress().toString().c_str());
            DeliveryResp(aucmd,smsc::system::Status::NOROUTE);
            registerEvent( stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
            if (session.get()) session->closeCurrentOperation();
            return;
        }
        routeId = ri.routeId;
        cmd->setDstEntity(dst);
        sms.setRouteId(ri.routeId);
        sms.setSourceSmeId(src->getSystemId());
        sms.setDestinationSmeId(dst->getSystemId());
        cmd->setServiceId(ri.serviceId);

        smsc_log_info( log_, "Delivery %s: %s USSD_OP=%d. %s(%s)->%s, routeId=%s",
                       cmd->getOperationId() != SCAGCommand::invalidOpId() ? "continued..." : "",
                       rcnt ? "(redirected)" : "", ussd_op,
                       sms.getOriginatingAddress().toString().c_str(),
                       src->getSystemId(),
                       sms.getDestinationAddress().toString().c_str(),
                       routeId.c_str());

        if ( ! session.get() ) {

            SessionKey key( sms.getOriginatingAddress() );
            session = sm.getSession( key, aucmd );
            if ( ! session.get() ) return; // locked

        }


        if ( ussd_op >= 0 && rcnt > 0 ) {
            smsc_log_warn(log_, "USSD Delivery: Rerouting for USSD dialog not allowed");
            DeliveryResp(aucmd, smsc::system::Status::NOROUTE);
            registerEvent( stat::events::smpp::REJECTED, src, dst, NULL, smsc::system::Status::NOROUTE);
            session->closeCurrentOperation();
            return;
        }

        SmppOperationMaker opmaker( *cmd, *session.get() );
        opmaker.setupOperation( st );
        if ( st.status != re::STATUS_OK ) {
            smsc_log_warn( log_, "Delivery op fail: %s, res=%d", opmaker.what(), st.result );
            DeliveryResp( aucmd, st.result );
            registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result );
            return;
        }

        smsc_log_debug(log_, "Delivery: RuleEngine processing...");
        re::RuleEngine::Instance().process(*cmd,*session.get(), st);
        smsc_log_debug(log_, "Delivery: RuleEngine processed: st.status=%d st.result=%d", st.status, st.result );

        opmaker.postProcess( st );

    } while (st.status == re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

    if (rcnt >= MAX_REDIRECT_CNT)
    {
        smsc_log_info(log_,"Delivery: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s",  sms.getOriginatingAddress().toString().c_str(),
                      src->getSystemId(), sms.getDestinationAddress().toString().c_str());
        DeliveryResp(aucmd,smsc::system::Status::NOROUTE);
        registerEvent( stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        session->closeCurrentOperation();
        return;
    }

    if (st.status == re::STATUS_LONG_CALL)
    {
        smsc_log_debug(log_,"Delivery: long call initiate");
        if(!SmppManager::Instance().makeLongCall(aucmd, session))
        {
            DeliveryResp(aucmd,smsc::system::Status::SYSERR);
            session->closeCurrentOperation();
            registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSERR);
        }
        return;
    }

    if(st.status != re::STATUS_OK)
    {
        smsc_log_info(log_,"Delivery: RuleEngine returned result=%d",st.result);
        if(!st.result)
        {
            smsc_log_warn(log_, "Delivery: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }
        DeliveryResp(aucmd,st.result);
        session->closeCurrentOperation();
        registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
        return;
    }

    if (smscmd.original_ussd_op != -1) // Not Sibinco USSD
        sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, smscmd.original_ussd_op);
    failed = putCommand(DELIVERY, src, dst, ri, aucmd);

    session->getLongCallContext().runPostProcessActions();
    smsc_log_debug(log_, "Delivery: processed");

    if(failed)
    {
        std::auto_ptr<SmppCommand> resp(SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),failed));
        resp->setEntity(dst);
        resp->get_resp()->setOrgCmd(aucmd.release());
        resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);
        processDeliveryResp(resp,session);
    }
}


void StateMachine::processDeliveryResp(std::auto_ptr<SmppCommand> aucmd, ActiveSession session )
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    int rs = -2;
    re::RuleStatus st;

    SmppEntity *dst, *src;
    SMS* sms;

    src = cmd->getEntity();

    do { // fake loop

        if( cmd->getOperationId() == SCAGCommand::invalidOpId() )
        {
            smsc_log_debug(log_, "DeliveryResp: got cmd=%p", cmd );
            SmppCommand* orgCmd;

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
                        smsc_log_warn(log_, "DeliveryResp: Src entity disconnected. sid='%s', seq='%d'",
                                      src->getSystemId(), cmd->get_dialogId());
                        //          registerEvent( stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
                        return;
                    }
                }

                std::auto_ptr<SmppCommand> auOrgCmd(reg_.Get(srcUid, cmd->get_dialogId()));
                orgCmd = auOrgCmd.get();
                if(!orgCmd)
                {
                    smsc_log_warn(log_,"%s: Original delivery for delivery response not found. sid='%s',seq='%d'",
                                  src ? "DeliveryResp" : "MSAG Receipt" , src ? src->getSystemId() : "NULL", cmd->get_dialogId());
                    return;
                } else if ( orgCmd->flagSet(SmppCommandFlags::NOTIFICATION_RECEIPT) ) {
                    smsc_log_debug(log_, "MSAG Receipt: Got responce, expired (srcuid='%d', seq='%d')", srcUid, cmd->get_dialogId());
                    return;
                }
                cmd->get_resp()->setOrgCmd(auOrgCmd.release());
            }
            else
            {
                orgCmd = cmd->get_resp()->getOrgCmd();
            }

            if(!orgCmd->get_smsCommand().essentialSlicedResponse(cmd->get_status() || cmd->get_resp()->expiredResp))
                return;

            dst = orgCmd->getEntity();
            cmd->setDstEntity(dst);
            sms = orgCmd->get_sms();

            // cmd->get_resp()->set_sms(sms);
            cmd->setServiceId(orgCmd->getServiceId());
            // will be done in opmaker
            // cmd->setOperationId(orgCmd->getOperationId());
            cmd->get_resp()->set_dir(dsdSrv2Sc);
            cmd->set_dialogId(orgCmd->get_smsCommand().get_orgDialogId());

            sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
            sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);
            
        } else {

            smsc_log_debug(log_, "DeliveryResp: continued... cmd=%p", cmd );
            dst = cmd->getDstEntity();
            sms = cmd->get_resp()->getOrgCmd()->get_sms();

        }

        getUSSDOp( "DeliveryResp", *sms, 0 );

        // session may be already locked (when failure resp is sent)
        if ( session.get() ) {
            session.moveLock(cmd);
            smsc_log_debug(log_,"DeliveryResp: got pre-locked session=%p", session.get() );
            break;
        }

        SessionKey key( sms->getOriginatingAddress() );
        session = SessionManager::Instance().getSession(key, aucmd);

        if ( ! session.get() ) return; // locked

        smsc_log_debug(log_,"DeliveryResp: got session=%p key='%s' %s",
                       session.get(), key.toString().c_str(), cmd->get_resp()->expiredResp ? "(expired)" : "");

    } while ( false ); // fake loop

    SmppOperationMaker opmaker( *cmd, *session.get() );
    opmaker.setupOperation( st );
    if ( st.status != re::STATUS_OK ) {
        smsc_log_warn( log_, "DeliveryResp op fail: %s, res=%d", opmaker.what(), st.result );
        registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)sms->getRouteId(), st.result );
    }

//    smsc_log_debug(log_, "sms:%x sm: %d mp: %d rsm: %d rmp: %d", sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE), sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD), sms->hasBinProperty(Tag::SMSC_RAW_SHORTMESSAGE), sms->hasBinProperty(Tag::SMSC_RAW_PAYLOAD));
    smsc_log_debug(log_, "DeliveryResp: RuleEngine processing...");
    re::RuleEngine::Instance().process(*cmd,*session.get(), st);
    smsc_log_debug(log_, "DeliveryResp: RuleEngine processed: st.status=%d st.result=%d", st.status, st.result );

    opmaker.postProcess( st );

    if (st.status == re::STATUS_LONG_CALL)
    {
  //          if (!reg_.Register(srcUid, cmd->get_dialogId(), orgCmd))
  //            throw Exception("DeliveryResp: Register cmd for uid=%d, seq=%d failed", dst->getUid(), cmd->get_dialogId());

        smsc_log_debug(log_,"DeliveryResp: long call initiate");
        if(SmppManager::Instance().makeLongCall(aucmd, session))
        {
            return;
        }
        rs = smsc::system::Status::SYSERR;
    }
    else if(st.status != re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log_, "DeliveryResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }
        rs = st.result;
    }

    // if(cmd.hasSession()) cmd.setSession(SessionPtr(NULL));
    cmd->get_resp()->set_messageId("");

    if(rs != -2)
    {
        cmd->set_status(rs);
        rs = stat::events::smpp::RESP_GW_REJECTED;
    }
    else if(cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
    {
        rs = stat::events::smpp::RESP_FAILED;
    }
    else if(cmd->get_resp()->expiredResp)
    {
        rs = stat::events::smpp::RESP_EXPIRED;
    }
    else if(cmd->get_status())
    {
        rs = stat::events::smpp::RESP_REJECTED;
    }
    else
    {
        rs = stat::events::smpp::RESP_OK;
    }

    registerEvent(rs, src, dst, (char*)sms->getRouteId(), rs != stat::events::smpp::RESP_OK ? cmd->get_status() : -1);

    try{
        dst->putCommand(aucmd);
    } catch(std::exception& e) {
        smsc_log_warn(log_,"DeliveryResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
    }

    if(session.get())
    {
        session->getLongCallContext().runPostProcessActions();
        // SessionManager::Instance().releaseSession(session);
    }
    // cmd->get_resp()->set_sms(0);
    smsc_log_debug(log_, "DeliveryResp: processed");
}


void StateMachine::SubmitResp(std::auto_ptr<SmppCommand> aucmd, int status )
{
    aucmd->getEntity()->putCommand
        ( SmppCommand::makeSubmitSmResp
          ( "0",
            aucmd->get_smsCommand().get_orgDialogId(),
            status,
            aucmd->get_sms()->getIntProperty(Tag::SMPP_DATA_SM)
            )
          );
}


void StateMachine::DeliveryResp( std::auto_ptr<SmppCommand> aucmd, int status )
{
    aucmd->getEntity()->putCommand
        ( SmppCommand::makeDeliverySmResp
          ( "0",
            aucmd->get_smsCommand().get_orgDialogId(),
            status
            )
          );
}


void StateMachine::DataResp( std::auto_ptr<SmppCommand> aucmd,int status)
{
    aucmd->getEntity()->putCommand
        ( SmppCommand::makeDataSmResp
          ( "0",
            aucmd->get_smsCommand().get_orgDialogId(),
            status
            )
          );
}


void StateMachine::processDataSm(std::auto_ptr<SmppCommand> aucmd)
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    smsc_log_debug(log_, "DataSm: got cmd=%p %s", aucmd.get(), cmd->getSession() ? "continued..." : "");
    uint32_t rcnt = 0, failed = 0;
    SmppEntity *src = NULL;
    SmppEntity *dst = NULL;
    ActiveSession session;
    router::RouteInfo ri;
    FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;
    re::RuleStatus st;
    SMS& sms=*(cmd->get_sms());
    SmsCommand& smscmd=cmd->get_smsCommand();
    SessionManager& sm = SessionManager::Instance();

    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src = cmd->getEntity();
    cmd->get_smsCommand().set_orgDialogId(cmd->get_dialogId());

    if( sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) )
    {
        smsc_log_info(log_,"DataSm: USSD dialog not allowed in DataSm %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
                        src->getSystemId(), sms.getDestinationAddress().toString().c_str());
        DataResp(aucmd,smsc::system::Status::USSDDLGREFMISM);
        registerEvent(stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::USSDDLGREFMISM);
        return;
    }

    do {

        dst=routeMan_->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
        smsc_log_debug(log_, "orig_route_id=%s, new_route_id=%s", routeId.c_str(), ri.routeId.c_str());
        if(!dst || routeId == ri.routeId)
        {
            smsc_log_info(log_,"DataSm: %s %s(%s)->%s", !dst ? "no route" : "redirection to the same route",
                          sms.getOriginatingAddress().toString().c_str(),
                          src->getSystemId(),
                          sms.getDestinationAddress().toString().c_str());
            DataResp(aucmd,smsc::system::Status::NOROUTE);
            registerEvent(stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
            if (session.get()) session->closeCurrentOperation();
            return;
        }
        routeId = ri.routeId;
        cmd->setDstEntity(dst);
        sms.setRouteId(ri.routeId);
        sms.setSourceSmeId(src->getSystemId());
        sms.setDestinationSmeId(dst->getSystemId());

        if(src->info.type==etService)
            smscmd.dir = (dst->info.type==etService) ? dsdSrv2Srv : dsdSrv2Sc;
        else
            smscmd.dir = (dst->info.type==etService) ? dsdSc2Srv : dsdSc2Sc;

        cmd->setServiceId(ri.serviceId);

        smsc_log_info( log_, "DataSm %s: %s. %s(%s)->%s, routeid=%s",
                       cmd->getOperationId() != SCAGCommand::invalidOpId() ? "continued..." : "",
                       rcnt ? "(redirected)" : "",
                       sms.getOriginatingAddress().toString().c_str(),
                       src->getSystemId(),
                       sms.getDestinationAddress().toString().c_str(),
                       routeId.c_str() );

        if (!session.get())
        {
            SessionKey key( (src->info.type == etService) ?
                            sms.getDestinationAddress() : sms.getOriginatingAddress() );
            session = sm.getSession( key, aucmd);
            if ( ! session.get() ) return; // locked
          
        }

        SmppOperationMaker opmaker( *cmd, *session.get() );
        opmaker.setupOperation( st );
        if ( st.status != re::STATUS_OK ) {
            smsc_log_warn( log_, "DataSm op fail: %s, res=%d", opmaker.what(), st.result );
            DataResp( aucmd, st.result );
            registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result );
            return;
        }
        
        smsc_log_debug(log_, "DataSm: RuleEngine processing...");
        re::RuleEngine::Instance().process(*cmd,*session.get(), st);
        smsc_log_debug(log_, "DataSm: RuleEngine processed: st.status=%d st.result=%d", st.status, st.result );

        opmaker.postProcess( st );

    } while ( st.status == re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT );

    if (rcnt >= MAX_REDIRECT_CNT)
    {
        smsc_log_info(log_,"DataSm: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s", sms.getOriginatingAddress().toString().c_str(),
                      src->getSystemId(), sms.getDestinationAddress().toString().c_str());
        DataResp(aucmd,smsc::system::Status::NOROUTE);
        session->closeCurrentOperation();
        registerEvent(stat::events::smpp::REJECTED, src, NULL, NULL, smsc::system::Status::NOROUTE);
        return;
    }

    if(st.status == re::STATUS_LONG_CALL)
    {
        smsc_log_debug(log_,"DataSm: long call initiate");
        if(!SmppManager::Instance().makeLongCall(aucmd, session))
        {
            DataResp(aucmd, smsc::system::Status::SYSERR);
            session->closeCurrentOperation();
            registerEvent(stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, smsc::system::Status::SYSERR);
        }
        return;
    }

    if (st.status != re::STATUS_OK)
    {
        smsc_log_info(log_,"DataSm: RuleEngine returned result=%d",st.result);
        if(!st.result)
        {
            smsc_log_warn(log_, "DataSm: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }
        DataResp(aucmd, st.result);
        session->closeCurrentOperation();
        registerEvent(stat::events::smpp::REJECTED, src, dst, (char*)ri.routeId, st.result);
        return;
    }

    failed = putCommand(DATASM, src, dst, ri, aucmd);

    session->getLongCallContext().runPostProcessActions();
    smsc_log_debug(log_, "DataSm: processed");

    if (failed)
    {
        std::auto_ptr<SmppCommand> resp(SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),failed));
        resp->setEntity(dst);
        resp->get_resp()->setOrgCmd(aucmd.release());
        resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);
        processDataSmResp(resp,session);
    }
}


void StateMachine::processDataSmResp(std::auto_ptr<SmppCommand> aucmd, ActiveSession session )
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    int rs = -2;
    re::RuleStatus st;

    SmppEntity *dst, *src;
    SMS* sms;

    src = cmd->getEntity();

    do { // fake loop

        if( cmd->getOperationId() == SCAGCommand::invalidOpId() )
        {
            smsc_log_debug(log_, "DataSmResp: got cmd=%p", cmd );

            SmppCommand* orgCmd;
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
                        smsc_log_warn(log_, "DataSmResp: Src entity disconnected. sid='%s', seq='%d'",
                                      src ? src->getSystemId() : "NULL", cmd->get_dialogId());
                        //          registerEvent(stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
                        return;
                    }
                }
                orgCmd = reg_.Get(srcUid, cmd->get_dialogId()).release();
                if( !orgCmd )
                {
                    smsc_log_warn(log_,"DataSmResp: Original datasm for datasm response not found. sid='%s',seq='%d'",src->getSystemId(),cmd->get_dialogId());
                    return;
                }
                cmd->get_resp()->setOrgCmd( orgCmd );
            }
            else
            {
                orgCmd = cmd->get_resp()->getOrgCmd();
            }

            if(!orgCmd->get_smsCommand().essentialSlicedResponse(cmd->get_status() || cmd->get_resp()->expiredResp))
                return;

            sms = orgCmd->get_sms();
            // cmd->get_resp()->set_sms(sms);

            SmsCommand& smscmd = orgCmd->get_smsCommand();
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

            cmd->setServiceId(orgCmd->getServiceId());
            // will be done in opmaker
            // cmd->setOperationId(orgCmd->getOperationId());
            sms->setOriginatingAddress(smscmd.orgSrc);
            sms->setDestinationAddress(smscmd.orgDst);

            dst = orgCmd->getEntity();
            cmd->setDstEntity(dst);
            cmd->set_dialogId( orgCmd->get_smsCommand().get_orgDialogId() );

        } else {

            smsc_log_debug(log_, "DataSmResp: continued... cmd=%p", cmd);
            dst = cmd->getDstEntity();
            sms = cmd->get_resp()->getOrgCmd()->get_sms();

        }

        if ( session.get() ) {
            session.moveLock(cmd);
            smsc_log_debug(log_,"DataSmResp: got pre-locked session=%p", session.get() );
            break;
        }

        SessionKey key( ( cmd->get_resp()->get_dir() == dsdSc2Srv ||
                          cmd->get_resp()->get_dir() == dsdSrv2Srv ) ?
                        sms->getDestinationAddress() : sms->getOriginatingAddress() );
        session = SessionManager::Instance().getSession(key, aucmd);
        if ( ! session.get() ) return; // locked

        smsc_log_debug( log_, "DataSmResp: got session=%p. key='%s' %s",
                        session.get(), key.toString().c_str(), cmd->get_resp()->expiredResp ? "(expired)" : "");

    } while ( false ); // fake loop

    SmppOperationMaker opmaker( *cmd, *session.get() );
    opmaker.setupOperation( st );
    if ( st.status != re::STATUS_OK ) {
        smsc_log_warn( log_, "DataSmResp op fail: %s, res=%d", opmaker.what(), st.result );
        registerEvent( stat::events::smpp::REJECTED, src, dst, (char*)sms->getRouteId(), st.result );
        return;
    }

    smsc_log_debug(log_, "DataSmResp: RuleEngine processing...");
    re::RuleEngine::Instance().process(*cmd,*session.get(), st);
    smsc_log_debug(log_, "DataSmResp: RuleEngine processed: st.status=%d st.result=%d", st.status, st.result );

    opmaker.postProcess( st );

    if (st.status == re::STATUS_LONG_CALL)
    {
        smsc_log_debug(log_,"DataSmResp: long call initiate");
        if(SmppManager::Instance().makeLongCall(aucmd, session))
        {
            return;
        }
        rs = smsc::system::Status::SYSERR;
    }
    else if(st.status != re::STATUS_OK)
    {
        if(!st.result)
        {
            smsc_log_warn(log_, "DataSmResp: Rule failed and no error(zero rezult) returned");
            st.result = smsc::system::Status::SYSERR;
        }
        rs = st.result;
    }

    /*
  if(cmd.hasSession())
  {
    cmd.setSession(SessionPtr(NULL));
  }
     */

    if (!(cmd->get_resp()->get_messageId()))
    {
        cmd->get_resp()->set_messageId("");
    }

    if(rs != -2)
    {
        cmd->set_status(rs);
        rs = stat::events::smpp::RESP_GW_REJECTED;
    }
    else if(cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
    {
        rs = stat::events::smpp::RESP_FAILED;
    }
    else if(cmd->get_resp()->expiredResp)
    {
        rs = stat::events::smpp::RESP_EXPIRED;
    }
    else if(cmd->get_status())
    {
        rs = stat::events::smpp::RESP_REJECTED;
    }
    else
    {
        rs = stat::events::smpp::RESP_OK;
    }

    registerEvent(rs, src, dst, (char*)sms->getRouteId(), rs != stat::events::smpp::RESP_OK ? cmd->get_status() : -1);


    try{
        dst->putCommand(aucmd);
    } catch(std::exception& e) {
        smsc_log_warn(log_,"DataSmResp: Failed to put command into %s:%s",dst->getSystemId(),e.what());
    }
    
    if (session.get())
    {
        session->getLongCallContext().runPostProcessActions();
        // sessions::SessionManager::Instance().releaseSession(session);
    }

    // cmd->get_resp()->set_sms(0);
    smsc_log_debug(log_, "DataSmResp: processed");
}


void StateMachine::processExpiredResps()
{
    {
        MutexGuard mg(expMtx_);
        if (expProc_) return;
        expProc_ = true;
    }
    try {

        while ( true )
        {
            int uid;
            std::auto_ptr<SmppCommand> cmd( reg_.getExpiredCmd(uid) );
            if ( ! cmd.get() ) break;

            switch( cmd->getCommandId())
            {
            case DELIVERY: {
                std::auto_ptr<SmppCommand> resp( SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT) );
                resp->get_resp()->expiredResp = true;
                resp->get_resp()->expiredUid = uid;
                resp->setEntity(routeMan_->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
                processDeliveryResp(resp);
                break;
            }
            case SUBMIT: {
                std::auto_ptr<SmppCommand> resp( SmppCommand::makeSubmitSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT,false) );
                resp->get_resp()->expiredResp=true;
                resp->get_resp()->expiredUid=uid;
                resp->setEntity(routeMan_->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
                processSubmitResp(resp);
                break;
            }
            case DATASM: {
                std::auto_ptr<SmppCommand> resp(SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT));
                resp->get_resp()->expiredResp=true;
                resp->get_resp()->expiredUid=uid;
                resp->setEntity(routeMan_->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
                processDataSmResp(resp);
                break;
            }
            default : {
                ::abort();
            }
            } // switch
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_, "exception in processExpiredResps: %s", e.what() );
    } catch (...) {
    }
    {
        MutexGuard mg(expMtx_);
        expProc_=false;
    }
}

    
void StateMachine::processAlertNotification( std::auto_ptr<SmppCommand> aucmd)
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    smsc_log_debug(log_, "AlertNotification: processing cmd=%p...", cmd );
  
    router::RouteInfo ri;
    SmppEntity *dst;
    dst = routeMan_->RouteSms( cmd->getEntity()->getSystemId(),
                               cmd->get_alertNotification().src,
                               cmd->get_alertNotification().dst,ri );
    if(!dst)
    {
        smsc_log_warn( log_,"Route not found for alert notification %s->%s",
                       cmd->get_alertNotification().src.toString().c_str(),
                       cmd->get_alertNotification().dst.toString().c_str());
        return;
    }
    try {
        dst->putCommand( aucmd );
    }catch(std::exception& e) {
        smsc_log_warn(log_,"Failed to putCommand alert notification:'%s'",e.what());
    }

    smsc_log_debug(log_, "AlertNotification: processed.");
}


void StateMachine::sendReceipt(std::auto_ptr<SmppCommand> aucmd)
{
    if ( ! aucmd.get() ) return;
    SmppCommand& cmd = * aucmd.get();
  SmppEntity *dst = cmd.getDstEntity();
  try{
      if(dst->getBindType() == btNone)
      {
        smsc_log_info(log_,"MSAG Receipt: sme not connected (%s)", dst->getSystemId());
      }
      else
      {
        int newSeq=dst->getNextSeq();
        if(!reg_.Register(dst->getUid(), newSeq, aucmd.get()))
          throw Exception("MSAG Receipt: Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
        dst->putCommand(aucmd);
      }
  } catch(std::exception& e)
  {
    smsc_log_info(log_, "MSAG Receipt: Failed to putCommand into %s:%s", dst->getSystemId(), e.what());
  }
}


int StateMachine::getUSSDOp( const char* where,
                             SMS&        sms,
                             SmsCommand* smscmd ) const
{
    int ussd_op = 
        sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ?
        sms.getIntProperty(Tag::SMPP_USSD_SERVICE_OP) : -1;
    if ( ussd_op == 35 ) // Not Sibinco USSD dialog
    {
        smsc_log_info( log_, "%s: Not Sibinco USSD dialog=%d for %s=>%s",
                       where, 
                       ussd_op,
                       sms.getDestinationAddress().toString().c_str(),
                       sms.getDestinationAddress().toString().c_str() );
        sms.dropProperty( Tag::SMPP_USSD_SERVICE_OP );
        if ( smscmd ) smscmd->original_ussd_op = ussd_op;
        ussd_op = -1;
    }
    return ussd_op;
}


}//smpp
}//transport
}//scag
