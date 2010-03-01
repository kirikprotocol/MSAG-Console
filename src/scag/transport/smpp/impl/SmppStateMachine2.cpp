#include <cassert>
#include "SmppManager2.h"
#include "SmppStateMachine2.h"
#include "scag/transport/smpp/common/SmppUtil.h"
#include "scag/transport/smpp/common/SmsSplit.h"
#include "core/buffers/XHash.hpp"
#include "scag/re/base/RuleEngine2.h"
#include "scag/re/base/ActionContext2.h"
#include "scag/sessions/base/SessionManager2.h"
#include "scag/sessions/base/Operation.h"
#include "scag/stat/base/Statistics2.h"
#include "system/status.h"
#include "scag/transport/smpp/router/route_types.h"
#include "scag/config/route/RouteStructures.h"
#include "SmppOperationMaker.h"
#include "scag/util/HRTimer.h"
#include "scag/re/base/CommandBridge.h"
#include "scag/re/base/EventHandlerType.h"
#include "scag/counter/Manager.h"
#include "scag/counter/Accumulator.h"

namespace {
using namespace scag2::transport::smpp;

bool isDirFromService( DataSmDirection dir ) {
    return ( dir == dsdSrv2Sc || dir == dsdSrv2Srv );
}

}

namespace scag2 {
namespace transport {
namespace smpp {

using namespace lcm;
using namespace sessions;
using namespace stat;
using namespace scag::transport::smpp;
using namespace smsc::core::buffers;
using scag2::re::actions::CommandProperty;

// std::vector<int> StateMachine::allowedUnknownOptionals;

const uint32_t MAX_REDIRECT_CNT = 10;
const unsigned TIMINGFREQ = 10000;

struct StateMachine::ResponseRegistry
{
    ResponseRegistry() : log(0), timeout(60) {}

    ~ResponseRegistry() {
        // FIXME: cleanup all items in reg
    }

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
            return (key.uid<<9) + key.seq;
        }
    };


    bool Register( int uid, int seq, SmppCommand* cmd, bool limit = true)
    {
        if (!cmd) return false;
        CommandId cmdid( CommandId(cmd->getCommandId()) );
        assert( cmdid == SUBMIT || cmdid == DELIVERY || cmdid == DATASM );

        RegKey key(uid, seq);
        sync::MutexGuard mg(mtx);
        if (!log) log=smsc::logger::Logger::getInstance("smpp.reg");
        if (reg.Exists(key))
        { // key already registered
            smsc_log_warn(log, "register %d/%d failed", uid, seq);
            return false;
        }
        if(cmd->getDstEntity()->info.outQueueLimit > 0)
        {
            const char* sysname = cmd->getDstEntity()->info.systemId.c_str();
            counter::CounterPtrAny* p = outCnt.GetPtr(sysname);
            // int* p=outCnt.GetPtr(cmd->getDstEntity()->info.systemId.c_str());
            if(limit && p && (*p)->getValue() > cmd->getDstEntity()->info.outQueueLimit)
            {
                smsc_log_warn(log,"command registration for '%s' denied: outQueueLimit",sysname);
                return false;
            }
            if(p)
            {
                (*p)->increment(); // (*p)++;
            }else
            {
                // outCnt.Insert(cmd->getDstEntity()->info.systemId.c_str(),1);
                const char* cname = "sys.smpp.queue.out";
                char buf[100];
                snprintf(buf,sizeof(buf),"%s.%s",cname,sysname);
                counter::Manager& mgr = counter::Manager::getInstance();
                counter::ObserverPtr o = mgr.getObserver(cname);
                counter::CounterPtrAny pp = 
                    mgr.registerAnyCounter(new counter::Accumulator(buf,o.get()));
                outCnt.Insert(sysname,pp);
                if (pp.get()) {
                    // pp->setMaxVal(cmd->getDstEntity()->info.outQueueLimit);
                    pp->increment();
                } else {
                    smsc_log_warn(log,"cannot create a counter '%s' for '%s'",cname,sysname);
                }
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
        smsc_log_debug(log, "register uid=%d, seq=%d, name=%s", uid, seq,
                       cmd->getDstEntity()->info.systemId.c_str());
        return true;
    }


    std::auto_ptr<SmppCommand> Get( int uid, int seq )
    {
        sync::MutexGuard mg(mtx);
        if (!log) log = smsc::logger::Logger::getInstance("smpp.reg");
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
                counter::CounterPtrAny& p = outCnt.Get(cmd->getDstEntity()->info.systemId.c_str());
                if (p.get()) p->increment(-1);
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
                counter::CounterPtrAny& p = outCnt.Get(cmd->getDstEntity()->info.systemId.c_str());
                if (p.get()) p->increment(-1);
            }

        } while ( false );
        return cmd;
    }

private:
    smsc::logger::Logger* log;
    int timeout;
    TimeOutList toList;
    XHash<RegKey, RegValue, HashFunc> reg;
    // Hash<int> outCnt;
    Hash< counter::CounterPtrAny > outCnt;
    sync::Mutex mtx;
};


StateMachine::ResponseRegistry StateMachine::reg_;

sync::Mutex StateMachine::expMtx_;
bool StateMachine::expProc_ = false;


int StateMachine::Execute()
{
    SmppCommand* cmd;
    std::string hrresult;
    hrresult.reserve(256);
    util::HRTiming hrt;
    unsigned passcount = 0;
    while(!isStopping)
    {
        try{

            while(queue_->getCommand(cmd))
            {
                if ( !cmd ) continue;
                std::auto_ptr<SmppCommand> aucmd(cmd);
                
                // smsc_log_debug(log_,"smppstm rolling");
                if ( cmd->getCommandId() != PROCESSEXPIREDRESP ) {
                    smsc_log_debug(log_,"Exec: processing cmd=%p serial=%u %d(%s) from %s",
                                   cmd,
                                   cmd->getSerial(),
                                   cmd->getCommandId(),
                                   commandIdName(cmd->getCommandId()),
                                   cmd->getEntity() ? cmd->getEntity()->getSystemId():"" );
                }

                const bool dotiming = ( ++passcount % TIMINGFREQ ) == 0;
                if ( dotiming ) hrt.reset( hrresult, 1000 );
                bool realdotiming = false;
                switch(cmd->getCommandId())
                {
                case SUBMIT: {
                    processSm(aucmd, dotiming ? &hrt : 0);
                    realdotiming = dotiming && hrt.isValid();
                    break;
                }
                case SUBMIT_RESP: {
                    processSmResp(aucmd);
                    break;
                }
                case DELIVERY: {
                    if ( cmd->flagSet(SmppCommandFlags::NOTIFICATION_RECEIPT) )
                        sendReceipt(aucmd);
                    else {
                        processSm(aucmd, dotiming ? &hrt : 0 );
                        realdotiming = dotiming && hrt.isValid();
                    }
                    break;
                }
                case DELIVERY_RESP:
                    processSmResp(aucmd);
                    break;
                case DATASM: {
                    processSm(aucmd, dotiming ? &hrt : 0);
                    realdotiming = dotiming && hrt.isValid();
                    break;
                }
                case DATASM_RESP:         processSmResp(aucmd);             break;
                case PROCESSEXPIREDRESP:  processExpiredResps();            break;
                case ALERT_NOTIFICATION:  processAlertNotification(aucmd);  break;
                default:
                    smsc_log_warn(log_,"Unprocessed command %p id %d", cmd, cmd->getCommandId());
                    break;
                }

                if ( realdotiming ) {
                    hrt.mark( "stm.endloop" );
                    smsc_log_info(log_, "timing:%s", hrt.result().c_str() );
                }
            }
        } catch(std::exception& e)
            {
                smsc_log_error(log_,"Exception in state machine:%s",e.what());
            }
    } // while
    return 0;
}


void StateMachine::registerEvent( int event, SmppEntity* src, SmppEntity* dst,
                                  const char* rid, int errCode )
{
    char* src_id;
    char* dst_id = NULL;
    bool srcType, dstType = false;

    smsc_log_debug(log_, "SmppStateMachine Event:%d", event);

    if (!src) {
        // cannot proceed
        smsc_log_error(log_,"SmppStateMachine: no src passed! The event is lost.");
        return;
    }
    src_id = (char*)src->info.systemId.c_str();
    srcType = src->info.type == etSmsc;

    if(dst)
    {
        dst_id = (char*)dst->info.systemId.c_str();
        dstType = dst->info.type == etSmsc;
    }

    Statistics::Instance().registerEvent
        ( SmppStatEvent(src_id, srcType, dst_id, dstType, rid, event, errCode) );
}

    
uint32_t StateMachine::putCommand(CommandId cmdType, SmppEntity* src, SmppEntity* dst, router::RouteInfo& ri,
                                  std::auto_ptr<SmppCommand>& aucmd )
{
    if ( ! aucmd.get() ) return smsc::system::Status::SYSFAILURE;
    SmppCommand* cmd = aucmd.get();
    const char *cmdName;
    switch ( cmdType ) {
    case SUBMIT :
        cmdName = "Submit";
        break;
    case DELIVERY:
        cmdName = "Delivery";
        break;
    case DATASM :
        cmdName = "DataSm";
        break;
    default:
        cmdName = 0;
    }
    uint32_t failed = 0;
    SMS& sms = *cmd->get_sms();

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
            // stripUnknownSmppOptionals( sms,allowedUnknownOptionals );

            uint32_t cnt;
            const bool sliced = isSliced(sms);

            if( ri.slicing != router::SlicingType::NONE && 
                !sliced && (cnt = getPartsCount(sms)) > 1)
            {
                smsc_log_debug(log_, "%s: slicing message, type=%d, parts=%d, resppolicy=%d", cmdName, ri.slicing, cnt, ri.slicingRespPolicy);
                cmd->get_smsCommand().setSlicingParams(ri.slicingRespPolicy, cnt);
                uint32_t seq = 0, refNum = dst->getNextSlicingSeq(ri.slicing);

                SMS partSms;
                uint8_t udhType = ri.slicing > router::SlicingType::SAR ?
                    ri.slicing - router::SlicingType::SAR : 0;
                while( (seq = getNextSmsPart(sms, partSms, refNum, seq, udhType, cmdType == DATASM)) )
                {
                    int newSeq=dst->getNextSeq();
                    std::auto_ptr<SmppCommand> partCmd( SmppCommand::makeCommandSm(cmdType, partSms, newSeq) );
                    if (!reg_.Register(dst->getUid(), newSeq, cmd, seq == 1))
                        throw smsc::util::Exception("%s: Register cmd for uid=%d, seq=%d failed", cmdName, dst->getUid(), newSeq);
                    dst->putCommand(partCmd);
                }
            }
            else
            {
                int newSeq=dst->getNextSeq();
                if ( sliced ) {
                    // we have to replace sms slicing refnum, to allow several
                    // sources to come into one destination.
                    uint32_t newsarmr = src->countSlicedOnOutput(dst,cmd);
                    if ( uint8_t(newsarmr/0x10000) != router::SlicingType::NONE ) {
                        // remember the original slicing ref num
                        // sms.setConcatMsgRef(uint16_t(sliceRefNum));
                        SmppCommand::changeSliceRefNum(sms,newsarmr);
                    }
                }
                if (!reg_.Register(dst->getUid(), newSeq, cmd))
                    throw smsc::util::Exception("%s: Register cmd for uid=%d, seq=%d failed", cmdName, dst->getUid(), newSeq);
                dst->putCommand(aucmd);
            }
            registerEvent( stat::events::smpp::ACCEPTED, src, dst, ri.routeId.c_str(), -1 );
        }
    } catch (std::exception& e) {
        failed = smsc::system::Status::SYSFAILURE;
        smsc_log_info(log_,"%s: Failed to putCommand into %s:%s", cmdName, dst->getSystemId(),e.what());
    }
    return failed;
}

void StateMachine::processSmResp( std::auto_ptr<SmppCommand> aucmd,
                                  ActiveSession session )
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    re::RuleStatus st;
    st.status = re::STATUS_OK;
    st.result = 0;
    DataSmDirection dir; // direction of the original command
    const char* where;
    switch (cmd->getCommandId()) {
    case SUBMIT_RESP :
        dir = dsdSrv2Sc;
        where = "SubmitResp";
        break;
    case DELIVERY_RESP:
        dir = dsdSc2Srv;
        where = "DeliveryResp";
        break;
    case DATASM_RESP :
        dir = dsdUnknown;
        where = "DataSmResp";
        break;
    default:
        smsc_log_error( log_, "processResp: wrong command cmd=%p cmd->serial=%u type=%d(%s)",
                        cmd,
                        cmd->getSerial(),
                        cmd->getCommandId(),
                        commandIdName(cmd->getCommandId()) );
        return;
    }

    // fix for session lockup
    SessionManager::Instance().restoreSession(session,cmd);

    SmppEntity *dst, *src;
    SMS* sms;

    src = cmd->getEntity();

    do { // fake loop

        smsc_log_debug(log_, "%s: got cmd=%p cmd->serial=%u cmd->sess=%p%s",
                       where,
                       cmd,
                       cmd->getSerial(),
                       cmd->getSession(),
                       cmd->getOperationId() != invalidOpId() ? ", continued..." : ""
                       );

        router::RouteInfo ri = router::RouteInfo();
        if ( cmd->getOperationId() == invalidOpId() )
        {
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
                        if (src) srcUid = src->getUid();
                    }
                    catch (std::exception& exc)
                    {
                        smsc_log_warn(log_, "%s: Src entity disconnected. sid='%s', seq='%d'",
                                      where,
                                      src ? src->getSystemId() : "NULL", cmd->get_dialogId());
                        return;
                    }
                }

                orgCmd = reg_.Get(srcUid, cmd->get_dialogId()).release();
                if( !orgCmd )
                {
                    smsc_log_warn(log_,"%s: Original cmd not found. sid='%s',seq='%d'",
                                  where, src ? src->getSystemId() : "NULL" ,cmd->get_dialogId() );
                    return;
                }
                cmd->get_resp()->setOrgCmd( orgCmd );
            }
            else
            {
                orgCmd = cmd->get_resp()->getOrgCmd();
            }

            if ( dir == dsdSc2Srv && orgCmd->flagSet(SmppCommandFlags::NOTIFICATION_RECEIPT) ) {
                // for delivery only
                smsc_log_debug(log_, "MSAG Receipt: Got responce, expired sid='%s', seq='%d'",
                               src ? src->getSystemId() : "NULL", cmd->get_dialogId());
                return;
            }

            if(!orgCmd->get_smsCommand().essentialSlicedResponse(cmd->get_status() || cmd->get_resp()->expiredResp))
                return;

            sms = orgCmd->get_sms();
            SmsCommand& smscmd = orgCmd->get_smsCommand();
            if ( dir == dsdUnknown ) dir = smscmd.dir;
            if (dir == dsdSrv2Sc) {
                cmd->get_resp()->set_dir(dsdSc2Srv);
            } else if (dir == dsdSc2Srv) {
                cmd->get_resp()->set_dir(dsdSrv2Sc);
            } else {
                cmd->get_resp()->set_dir(dir);
            }

            // const bool keyisdest = ( dir == dsdSrv2Sc || dir == dsdSrv2Srv ) ? true : false;

            cmd->setServiceId(orgCmd->getServiceId());
            sms->setOriginatingAddress(smscmd.orgSrc);
            sms->setDestinationAddress(smscmd.orgDst);
            dst = orgCmd->getEntity();
            if (!dst) {
                smsc_log_warn(log_,"orgcmd has no src for resp sid='%s' seq=%u",
                              src ? src->getSystemId() : "NULL", cmd->get_dialogId() );
                return;
            }
            cmd->setDstEntity(dst);
            cmd->set_dialogId( smscmd.get_orgDialogId() );

            smscmd.getRouteInfo(ri);

            // orgcmd->operation is not set in case of transit route
            if ( orgCmd->getOperationId() == invalidOpId() ) {
                smsc_log_debug(log_, "%s: orgcmd has no operation, transit route?", where );
                if (ri.statistics) {
                  const Address& address =  isDirFromService(dir) ? sms->getDestinationAddress() : sms->getOriginatingAddress();
                  const SessionKey key( address );
                  CommandProperty cp(scag2::re::CommandBridge::getCommandProperty(*cmd, address, static_cast<uint8_t>(cmd->getOperationId())));
                  SessionPrimaryKey primaryKey(key);
                  timeval tv = { time(0), 0 };
                  primaryKey.setBornTime(tv);
                  smsc_log_debug(log_, "%s: register traffic info event for transit route", where);
                  scag2::re::CommandBridge::RegisterTrafficEvent(cp, primaryKey, "");
                }
                break;
            }

        } else {

            dst = cmd->getDstEntity();
            sms = cmd->get_resp()->getOrgCmd()->get_sms();

        }

        // ussdop fixup for submit/delivery only
        if ( cmd->getCommandId() != DATASM_RESP ) getUSSDOp( where, *sms, 0 );

        if ( session.get() ) {
            session.moveLock(cmd);
            smsc_log_debug(log_,"%s: got pre-locked session=%p", where, session.get() );

        } else {

            // no session
            const SessionKey key( ( dir == dsdSrv2Sc || dir == dsdSrv2Srv ) ?
                                  sms->getDestinationAddress() :
                                  sms->getOriginatingAddress() );
            // NOTE: session should already exist, so if it is not found
            // it means that session has been expired.
            session = SessionManager::Instance().getSession(key, aucmd, false);
            if ( ! aucmd.get() ) {
                // command is taken, session is locked
                return;
            } else if ( ! session.get() ) {
                // session is not found
                smsc_log_warn(log_,"%s: session is not found, expired? key='%s'",
                              where,
                              key.toString().c_str() );
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::TRANSACTIONTIMEDOUT;
                break;
            }


            smsc_log_debug(log_,"%s: got session=%p key='%s' %s",
                           where,
                           session.get(), key.toString().c_str(), cmd->get_resp()->expiredResp ? "(expired)" : "");

        } // if no pre-locked session

        // session should be here

        // create operation
        const Address& address =  isDirFromService(dir) ? sms->getDestinationAddress() : sms->getOriginatingAddress();
        CommandProperty cp(scag2::re::CommandBridge::getCommandProperty(*cmd, address, static_cast<uint8_t>(cmd->getOperationId())));
        SmppOperationMaker opmaker( where, aucmd, session, log_ );
        opmaker.process( st, cp );

        // NOTE: long call will grab the session!
        if ( st.status == re::STATUS_LONG_CALL ) return;

        // if (ri.statistics && !session->getLongCallContext().continueExec) {
        if ( ri.statistics ) {
          smsc_log_debug(log_, "%s: register traffic info event, keywords='%s'", where, cp.keywords.c_str());
          scag2::re::CommandBridge::RegisterTrafficEvent(cp, session->sessionPrimaryKey(), "");
        }

    } while ( false ); // fake loop

    if ( cmd->getCommandId() == DELIVERY_RESP ||
         !(cmd->get_resp()->get_messageId()) ) {
        cmd->get_resp()->set_messageId("");
    }

    int staterr;
    int err = int(cmd->get_resp()->getOrigStatus());
    if ( err ) {
        staterr = stat::events::smpp::RESP_REJECTED;
    } else if ( st.result ) {
        cmd->set_status( err = st.result );
        staterr = stat::events::smpp::RESP_GW_REJECTED;
    } else {
        err = cmd->get_status();
        if (cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
        {
            staterr = stat::events::smpp::RESP_FAILED;
        }
        else if(cmd->get_resp()->expiredResp)
        {
            staterr = stat::events::smpp::RESP_EXPIRED;
        }
        else
        {
            err = -1;
            staterr = stat::events::smpp::RESP_OK;
        }
    }

    registerEvent( staterr, src, dst, (char*)sms->getRouteId(), err );

    try{
        dst->putCommand(aucmd);
    } catch(std::exception& e) {
        smsc_log_warn(log_,"%s: Failed to put command into %s:%s",
                      where, dst->getSystemId(),e.what());
    }
    
    if (session.get())
    {
        session->getLongCallContext().runPostProcessActions();
    }
    smsc_log_debug(log_, "%s: processed", where );
}


void StateMachine::processSm( std::auto_ptr<SmppCommand> aucmd, util::HRTiming* inhrt )
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;

    const char* where;
    switch (cmd->getCommandId()) {
    case SUBMIT :
        where = "Submit";
        break;
    case DELIVERY :
        where = "Delivery";
        break;
    case DATASM :
        where = "DataSm";
        break;
    default :
        ::abort();
    }

    util::HRTiming hrt( inhrt );
    hrt.comment( where );

    smsc_log_debug(log_, "%s: got cmd=%p cmd->serial=%u cmd->sess=%p%s",
                   where,
                   cmd,
                   cmd->getSerial(),
                   cmd->getSession(),
                   cmd->getOperationId() != invalidOpId() ? ", continued..." : "" );
    uint32_t rcnt = 0, failed = 0;
    SmppEntity *src = NULL;
    SmppEntity *dst = NULL;
    ActiveSession session;
    router::RouteInfo ri;
    FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;
    re::RuleStatus st;
    st.status = re::STATUS_REDIRECT; // to start the loop

    SMS& sms = *cmd->get_sms();
    SmsCommand& smscmd = cmd->get_smsCommand();
    SessionManager& sm = SessionManager::Instance();
    smscmd.set_orgDialogId(cmd->get_dialogId());

    // fix for session lockup
    SessionManager::Instance().restoreSession(session,cmd);

    int ussd_op = -1;
    do {
        if ( cmd->getCommandId() == DATASM ) {
            if ( sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ) {
                smsc_log_info( log_, "%s: USSD dialog not allowed in DataSm %s(%s)->%s",
                               where, sms.getOriginatingAddress().toString().c_str(),
                               src->getSystemId(), 
                               sms.getDestinationAddress().toString().c_str());
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::USSDDLGREFMISM;
                break;
            }
        } else {
            ussd_op = getUSSDOp( where, sms, &smscmd );
            smscmd.dir = cmd->getCommandId() == SUBMIT ? dsdSrv2Sc : dsdSc2Srv;
        }
        smscmd.orgSrc=sms.getOriginatingAddress();
        smscmd.orgDst=sms.getDestinationAddress();
    } while ( false );

    src = cmd->getEntity();
    bool routeset = smscmd.hasRouteSet();
    int statevent = stat::events::smpp::GW_REJECTED;

    while ( st.status == re::STATUS_REDIRECT ) {

        st.status = re::STATUS_OK;
        st.result = 0;

        if ( ! routeset ) {

            if ( rcnt > 0 && ussd_op >= 0 ) {
                smsc_log_warn(log_, "%s(USSD): Rerouting for USSD dialog not allowed",
                              where );
                ri = router::RouteInfo();
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::NOROUTE;
                hrt.stop();
                break;
            }

            dst = routeMan_->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
            smsc_log_debug(log_, "%s: orig_route_id=%s, new_route_id=%s",
                           where, routeId.c_str(), ri.routeId.c_str());
            {
                const char* fail = 0;
                if (!dst) {
                    fail = "no route";
                    st.result = smsc::system::Status::NOROUTE;
                } else if ( routeId == ri.routeId ) {
                    fail = "redirect to the same route";
                    st.result = smsc::system::Status::NOROUTE;
                } else if ( dst->getBindType() == btNone ) {
                    fail = "sme not connected";
                    routeId = ri.routeId;
                    st.result = smsc::system::Status::SMENOTCONNECTED;
                    statevent = stat::events::smpp::REJECTED;
                }

                if ( fail ) {
                    smsc_log_info(log_,"%s: %s %s(%s)->%s",
                                  where,
                                  fail,
                                  sms.getOriginatingAddress().toString().c_str(),
                                  src->getSystemId(),
                                  sms.getDestinationAddress().toString().c_str());
                    st.status = re::STATUS_FAILED;
                    hrt.stop();
                    break;
                }
            }
            routeId = ri.routeId;
            cmd->setDstEntity(dst);
            sms.setRouteId(ri.routeId.c_str());
            sms.setSourceSmeId(src->getSystemId());
            sms.setDestinationSmeId(dst->getSystemId());
            cmd->setServiceId(ri.serviceId);
            
            if ( cmd->getCommandId() == DATASM ) {
                if(src->info.type==etService)
                    smscmd.dir = (dst->info.type==etService) ? dsdSrv2Srv : dsdSrv2Sc;
                else
                    smscmd.dir = (dst->info.type==etService) ? dsdSc2Srv : dsdSc2Sc;
            }

        } else {
            smscmd.getRouteInfo( ri );
            dst = cmd->getDstEntity();
            routeId = sms.getRouteId();
        }

        hrt.mark( "stm.route" );

        smsc_log_debug( log_, "%s%s: %s, USSD_OP=%d. %s(%s)->%s, routeId=%s%s",
                       where,
                       cmd->getOperationId() != invalidOpId() ? " continued..." : "",
                       rcnt ? "(redirected)" : "", ussd_op,
                       sms.getOriginatingAddress().toString().c_str(),
                       src->getSystemId(),
                       sms.getDestinationAddress().toString().c_str(),
                       routeId.c_str(),
                       ri.transit ? "(transit)" : "");

        if ( ! routeset ) smscmd.setRouteInfo( ri );
        const bool keyisdest = ( cmd->getCommandId() == SUBMIT ? true :
                           ( cmd->getCommandId() == DELIVERY ? false :
                             ( src->info.type == etService ? true : false )));
        const Address& address =  keyisdest ? sms.getDestinationAddress() : sms.getOriginatingAddress();
        const SessionKey key( address );

        CommandProperty cp(scag2::re::CommandBridge::getCommandProperty(*cmd, address, static_cast<uint8_t>(cmd->getOperationId())));

        if ( ri.transit ) {
          if (ri.statistics) {
            SessionPrimaryKey primaryKey(key);
            timeval tv = { time(0), 0 };
            primaryKey.setBornTime(tv);
            smsc_log_debug(log_, "%s: register traffic info event for transit route", where);
            scag2::re::CommandBridge::RegisterTrafficEvent(cp, primaryKey, scag2::re::CommandBridge::getMessageBody(*cmd), &hrt);
          }
          hrt.stop();
          break;
        }

        if ( ! session.get() ) {
          /*
            if ( ! routeset ) smscmd.setRouteInfo( ri ); // in case session is locked
            
            bool keyisdest = ( cmd->getCommandId() == SUBMIT ? true :
                               ( cmd->getCommandId() == DELIVERY ? false :
                                 ( src->info.type == etService ? true : false )));
            const SessionKey key( keyisdest ? sms.getDestinationAddress() : sms.getOriginatingAddress() );
            */
            session = sm.getSession( key, aucmd, ! routeset ); // NOTE: create session only if not from longcall
            if ( ! session.get() ) {
                __require__( ! routeset );
                hrt.stop();
                return; // locked
            }
        }

        hrt.mark( "stm.getsess" );

        SmppOperationMaker opmaker( where, aucmd, session, log_ );
        opmaker.process( st, cp, &hrt );
        if ( st.status == re::STATUS_LONG_CALL ) {
            smscmd.setRouteInfo( ri );
            hrt.stop();
            return;
        }

        if ( hrt.mark("stm.rerule") > 100000 ) hrt.comment("/HRTWARN");

        routeset = false; // in case rerouting happens

        if ( rcnt++ >= MAX_REDIRECT_CNT ) {
            smsc_log_info(log_,"%s: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s",
                          where,
                          sms.getOriginatingAddress().toString().c_str(),
                          src->getSystemId(),
                          sms.getDestinationAddress().toString().c_str());
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::NOROUTE;
            break;
        }

        if (st.status == re::STATUS_OK && ri.statistics) {
          smsc_log_debug(log_, "%s: register traffic info event, keywords='%s'", where, cp.keywords.c_str());
          scag2::re::CommandBridge::RegisterTrafficEvent(cp, session->sessionPrimaryKey(), scag2::re::CommandBridge::getMessageBody(*cmd), &hrt);
        }

    } // while

    if (st.status != re::STATUS_OK )
    {
        std::auto_ptr< SmppCommand > resp;
        switch (cmd->getCommandId()) {
        case SUBMIT :
            resp = SmppCommand::makeSubmitSmResp
                ("0",smscmd.get_orgDialogId(),st.result,sms.getIntProperty(Tag::SMPP_DATA_SM));
            break;
        case DELIVERY :
            resp = SmppCommand::makeDeliverySmResp( "0", smscmd.get_orgDialogId(), st.result );
            break;
        case DATASM :
            resp = SmppCommand::makeDataSmResp("0", smscmd.get_orgDialogId(), st.result);
            break;
        default:
            break;
        }
        if (resp.get()) src->putCommand( resp );
        registerEvent( statevent, src, dst, routeId.c_str(), st.result );
        if ( session.get() ) session->closeCurrentOperation();
        hrt.stop();
        return;
    }

    if ( cmd->getCommandId() != DATASM ) {
        if (smscmd.original_ussd_op != -1) // Not Sibinco USSD
            sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, smscmd.original_ussd_op);
    }
    failed = putCommand(CommandId(cmd->getCommandId()), src, dst, ri, aucmd);

    hrt.mark("stm.putcmd");

    if (session.get()) session->getLongCallContext().runPostProcessActions();

    smsc_log_debug(log_, "%s: processed, fail=%d", where, failed ? 1 : 0 );

    if (failed)
    {
        hrt.comment("/failed");
        std::auto_ptr<SmppCommand> resp;
        switch (cmd->getCommandId()) {
        case SUBMIT:
            resp = SmppCommand::makeSubmitSmResp("0",cmd->get_dialogId(),failed);
            break;
        case DELIVERY:
            resp = SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),failed);
            break;
        case DATASM:
            resp = SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),failed);
            break;
        default :
            break;
        }
        resp->setEntity(dst);
        resp->get_resp()->setOrgCmd( aucmd.release() );
        resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);
        processSmResp(resp,session);
    }
}


/*
void StateMachine::processSubmitResp(std::auto_ptr<SmppCommand> aucmd, ActiveSession session )
{
    static const char* where = "SubmitResp";
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    re::RuleStatus st;
    st.status = re::STATUS_OK;
    st.result = 0;

    SmppEntity *dst, *src;
    SMS *sms;

    src = cmd->getEntity();

    do { // fake loop

        smsc_log_debug(log_, "%s: got cmd=%p cmd->serial=%u cmd->sess=%p%s",
                       where,
                       cmd,
                       cmd->getSerial(),
                       cmd->getSession(),
                       cmd->getOperationId() != invalidOpId() ? ", continued..." : ""
                       );

        if ( cmd->getOperationId() == invalidOpId() ) {

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
                        smsc_log_warn(log_, "%s: Src entity disconnected. sid='%s', seq='%d'",
                                      where,
                                      src->getSystemId(), cmd->get_dialogId());
                        //          registerEvent( stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
                        return;
                    }
                }

                std::auto_ptr<SmppCommand> auOrgCmd(reg_.Get(srcUid, cmd->get_dialogId()));
                orgCmd = auOrgCmd.get();
                if(! orgCmd )
                {
                    smsc_log_warn(log_,"%s: Original cmd not found. sid='%s',seq='%d'",
                                  where,
                                  src ? src->getSystemId() : "NULL", cmd->get_dialogId());
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

            sms = orgCmd->get_sms();
            SmsCommand& smscmd = orgCmd->get_smsCommand();
            cmd->get_resp()->set_dir(dsdSc2Srv);

            dst = orgCmd->getEntity();
            cmd->setDstEntity(dst);
            cmd->setServiceId( orgCmd->getServiceId() );
            cmd->set_dialogId(smscmd.get_orgDialogId());
            sms->setOriginatingAddress(smscmd.orgSrc);
            sms->setDestinationAddress(smscmd.orgDst);

            // orgcmd->operation is not set in case of transit route
            if ( orgCmd->getOperationId() == invalidOpId() ) {
                smsc_log_debug(log_, "%s: orgcmd has no operation, transit route?", where );
                break;
            }

        } else {

            dst = cmd->getDstEntity();
            sms = cmd->get_resp()->getOrgCmd()->get_sms();

        }

        getUSSDOp( where, *sms, 0 );

        if ( session.get() ) {
            // if session is already locked
            session.moveLock(cmd);
            smsc_log_debug(log_,"%s: got pre-locked session=%p", where, session.get() );

        } else {

            // no session
            SessionKey key( sms->getDestinationAddress() );
            // NOTE: session should be already there, so if it is not found
            // it means that session has been expired.
            session = SessionManager::Instance().getSession(key, aucmd, false);
            if ( !aucmd.get() ) {
                // command is taken, it seems that session is locked
                return;

            } else if ( ! session.get() ) {
                // session is not found
                smsc_log_warn(log_,"%s: session is not found, expired? key='%s'",
                              where,
                              key.toString().c_str() );
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::TRANSACTIONTIMEDOUT;
                break;
            }

            smsc_log_debug(log_,"%s: got session=%p key='%s' %s",
                           where,
                           session.get(), key.toString().c_str(), cmd->get_resp()->expiredResp ? "(expired)" : "");

        }

        // session should be here

        // create operation
        SmppOperationMaker opmaker( where, aucmd, session, log_ );
        opmaker.process( st );
        if ( st.status == re::STATUS_LONG_CALL ) return;

    } while ( false ); // fake loop

    if (!(cmd->get_resp()->get_messageId())) {
        smsc_log_warn(log_, "%s: messageId is null", where);
        cmd->get_resp()->set_messageId("");
    }

    int staterr;
    int err = int(cmd->get_resp()->getOrigStatus());
    if ( err ) {
        staterr = stat::events::smpp::RESP_REJECTED;
    } else if ( st.result ) {
        cmd->set_status( err = st.result );
        staterr = stat::events::smpp::RESP_GW_REJECTED;
    } else {
        err = cmd->get_status();
        if (cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
        {
            staterr = stat::events::smpp::RESP_FAILED;
        }
        else if (cmd->get_resp()->expiredResp)
        {
            staterr = stat::events::smpp::RESP_EXPIRED;
        }
        // else if(cmd->get_status())
        // {
        // staterr = stat::events::smpp::RESP_REJECTED;
        // }
        else
        {
            err = -1;
            staterr = stat::events::smpp::RESP_OK;
        }
    }

    registerEvent(staterr, src, dst, (char*)sms->getRouteId(), err);

    try {
        dst->putCommand(aucmd);
    } catch(std::exception& e) {
        smsc_log_warn( log_,"%s: Failed to put command into %s:%s",
                       where, dst->getSystemId(),e.what());
    }

    if (session.get())
    {
        session->getLongCallContext().runPostProcessActions();
    }
    smsc_log_debug(log_, "%s: processed", where );
}
 */


/*
void StateMachine::processDelivery(std::auto_ptr<SmppCommand> aucmd, util::HRTiming* inhrt )
{
    const char* where = "Dlvery";
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;

    util::HRTiming hrt( inhrt );
    hrt.comment( where );

    smsc_log_debug(log_, "%s: got cmd=%p cmd->serial=%u cmd->sess=%p%s",
                   where,
                   cmd,
                   cmd->getSerial(),
                   cmd->getSession(),
                   cmd->getOperationId() != invalidOpId() ? ", continued..." : "" );
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

    const int ussd_op = getUSSDOp(where, sms, &smscmd );
    smscmd.dir = dsdSc2Srv;
    smscmd.orgSrc=sms.getOriginatingAddress();
    smscmd.orgDst=sms.getDestinationAddress();
    src = cmd->getEntity();

    bool routeset = smscmd.hasRouteSet();
    int statevent = stat::events::smpp::GW_REJECTED;

    do { // rerouting loop

        st.status = re::STATUS_OK;
        st.result = 0;

        if ( ! routeset ) {

            if ( ussd_op >= 0 && rcnt > 0 ) {
                smsc_log_warn(log_, "%s (USSD): Rerouting for USSD dialog not allowed",
                              where );
                ri = router::RouteInfo();
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::NOROUTE;
                hrt.stop();
                break;
            }

            dst=routeMan_->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
            smsc_log_debug(log_, "%s: orig_route_id=%s, new_route_id=%s",
                           where, routeId.c_str(), ri.routeId.c_str());
            {
                const char* fail = 0;
                if (!dst) {
                    fail = "no route";
                    st.result = smsc::system::Status::NOROUTE;
                } else if ( routeId == ri.routeId ) {
                    fail = "redirect to the same route";
                    st.result = smsc::system::Status::NOROUTE;
                } else if ( dst->getBindType() == btNone ) {
                    fail = "sme not connected";
                    routeId = ri.routeId;
                    st.result = smsc::system::Status::SMENOTCONNECTED;
                    statevent = stat::events::smpp::REJECTED;
                }
                
                if ( fail ) {

                    smsc_log_info(log_,"%s: %s %s(%s)->%s",
                                  where,
                                  fail,
                                  sms.getOriginatingAddress().toString().c_str(),
                                  src->getSystemId(),
                                  sms.getDestinationAddress().toString().c_str());
                    st.status = re::STATUS_FAILED;
                    hrt.stop();
                    break;
                }
            }
            routeId = ri.routeId;
            cmd->setDstEntity(dst);
            sms.setRouteId(ri.routeId);
            sms.setSourceSmeId(src->getSystemId());
            sms.setDestinationSmeId(dst->getSystemId());
            cmd->setServiceId(ri.serviceId);

        } else {
            smscmd.getRouteInfo( ri );
            dst = cmd->getDstEntity();
            routeId = sms.getRouteId();
        }

        hrt.mark("stm.route");

        smsc_log_debug( log_, "%s%s: %s USSD_OP=%d. %s(%s)->%s, routeId=%s%s",
                       where,
                       cmd->getOperationId() != invalidOpId() ? " continued..." : "",
                       rcnt ? "(redirected)" : "", ussd_op,
                       sms.getOriginatingAddress().toString().c_str(),
                       src->getSystemId(),
                       sms.getDestinationAddress().toString().c_str(),
                       routeId.c_str(),
                       ri.transit ? "(transit)" : "");

        if ( ri.transit ) {
            hrt.stop();
            break;
        }

        if ( ! session.get() ) {
            if ( ! routeset ) smscmd.setRouteInfo( ri );
            SessionKey key( sms.getOriginatingAddress() );
            session = sm.getSession( key, aucmd, ! routeset );
            if ( ! session.get() ) {
                __require__( ! routeset );
                return; // locked
            }
        }

        hrt.mark("stm.getsess");

        SmppOperationMaker opmaker( where, aucmd, session, log_ );
        opmaker.process( st, &hrt );
        if ( st.status == re::STATUS_LONG_CALL ) {
            smscmd.setRouteInfo( ri );
            hrt.stop();
            return;
        }

        if ( hrt.mark("stm.rerule") > 100000 ) hrt.comment("/HRTWARN");

        routeset = false;

    } while ( st.status == re::STATUS_REDIRECT && rcnt++ < MAX_REDIRECT_CNT);

    if (rcnt >= MAX_REDIRECT_CNT)
    {
        smsc_log_info(log_,"%s: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s",
                      where,
                      sms.getOriginatingAddress().toString().c_str(),
                      src->getSystemId(), 
                      sms.getDestinationAddress().toString().c_str());
        st.status = re::STATUS_FAILED;
        st.result = smsc::system::Status::NOROUTE;
    }

    if (st.status != re::STATUS_OK)
    {
        src->putCommand
            ( SmppCommand::makeDeliverySmResp( "0", smscmd.get_orgDialogId(), st.result ));
        registerEvent(statevent, src, dst, (char*)routeId, st.result);
        if ( session.get() ) session->closeCurrentOperation();
        hrt.stop();
        return;
    }

    if (smscmd.original_ussd_op != -1) // Not Sibinco USSD
        sms.setIntProperty(Tag::SMPP_USSD_SERVICE_OP, smscmd.original_ussd_op);
    failed = putCommand(DELIVERY, src, dst, ri, aucmd);

    hrt.mark("stm.putcmd");

    if (session.get()) session->getLongCallContext().runPostProcessActions();

    smsc_log_debug(log_, "%s: processed", where );

    if (failed)
    {
        hrt.comment("/failed");
        std::auto_ptr<SmppCommand> resp(SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),failed));
        resp->setEntity(dst);
        resp->get_resp()->setOrgCmd(aucmd.release());
        resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);
        processSmResp(resp,session);
    }
}
 */


/*
void StateMachine::processDeliveryResp( std::auto_ptr<SmppCommand> aucmd,
                                        ActiveSession session )
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    re::RuleStatus st;
    st.status = re::STATUS_OK;
    st.result = 0;

    SmppEntity *dst, *src;
    SMS* sms;

    src = cmd->getEntity();
    const char* where = "DeliveryResp"; // src ? "DeliveryResp" : "MSAGReceipt";

    do { // fake loop

        smsc_log_debug(log_, "%s: got cmd=%p cmd->serial=%u cmd->sess=%p%s",
                       where,
                       cmd,
                       cmd->getSerial(),
                       cmd->getSession(),
                       cmd->getOperationId() != invalidOpId() ? ", continued..." : ""
                       );

        if ( cmd->getOperationId() == invalidOpId() ) {

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
                        smsc_log_warn(log_, "%s: Src entity disconnected. sid='%s', seq='%d'",
                                      where,
                                      src->getSystemId(), cmd->get_dialogId());
                        //          registerEvent( stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
                        return;
                    }
                }

                std::auto_ptr<SmppCommand> auOrgCmd(reg_.Get(srcUid, cmd->get_dialogId()));
                orgCmd = auOrgCmd.get();
                if(!orgCmd)
                {
                    smsc_log_warn(log_,"%s: Original cmd not found. sid='%s',seq='%d'",
                                  where,
                                  src ? src->getSystemId() : "NULL", cmd->get_dialogId());
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
            cmd->setServiceId(orgCmd->getServiceId());
            cmd->get_resp()->set_dir(dsdSrv2Sc);
            cmd->set_dialogId(orgCmd->get_smsCommand().get_orgDialogId());
            sms->setOriginatingAddress(orgCmd->get_smsCommand().orgSrc);
            sms->setDestinationAddress(orgCmd->get_smsCommand().orgDst);
            
            // orgcmd->operation is not set in case of transit route
            if ( orgCmd->getOperationId() == invalidOpId() ) {
                smsc_log_debug(log_, "%s: orgcmd has no operation, transit route?", where );
                break;
            }

        } else {

            dst = cmd->getDstEntity();
            sms = cmd->get_resp()->getOrgCmd()->get_sms();

        }

        getUSSDOp( where, *sms, 0 );

        if ( session.get() ) {
            // if session is already locked
            session.moveLock(cmd);
            smsc_log_debug(log_,"%s: got pre-locked session=%p", where, session.get() );

        } else {

            // no session
            SessionKey key( sms->getOriginatingAddress() );
            // NOTE: session should be already there, so if it is not found
            // it means that session has been expired.
            session = SessionManager::Instance().getSession(key, aucmd, false);
            if ( !aucmd.get() ) {
                // command is taken, it seems that session is locked
                return;

            } else if ( ! session.get() ) {
                // session is not found
                smsc_log_warn(log_,"%s: session is not found, expired? key='%s'",
                              where,
                              key.toString().c_str() );
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::TRANSACTIONTIMEDOUT;
                break;
            }

            smsc_log_debug(log_,"%s: got session=%p key='%s' %s",
                           where,
                           session.get(), key.toString().c_str(), cmd->get_resp()->expiredResp ? "(expired)" : "");

        } // if no pre-locked session

        // session should be here

        // create operation
        SmppOperationMaker opmaker( where, aucmd, session, log_ );
        opmaker.process( st );
        if ( st.status == re::STATUS_LONG_CALL ) return;

    } while ( false ); // fake loop


    // if(cmd.hasSession()) cmd.setSession(SessionPtr(NULL));
    cmd->get_resp()->set_messageId("");

    int staterr;
    int err = int(cmd->get_resp()->getOrigStatus());
    if ( err ) {
        staterr = stat::events::smpp::RESP_REJECTED;
    } else if ( st.result ) {
        cmd->set_status( err = st.result );
        staterr = stat::events::smpp::RESP_GW_REJECTED;
    } else {
        err = cmd->get_status();
        if ( cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP)) {
            staterr = stat::events::smpp::RESP_FAILED;
        } else if (cmd->get_resp()->expiredResp) {
            staterr = stat::events::smpp::RESP_EXPIRED;
        }
        else
        {
            err = -1;
            staterr = stat::events::smpp::RESP_OK;
        }
    }

    registerEvent( staterr, src, dst, (char*)sms->getRouteId(), err);

    try{
        dst->putCommand(aucmd);
    } catch(std::exception& e) {
        smsc_log_warn( log_,"%s: Failed to put command into %s:%s",
                       where, dst->getSystemId(),e.what());
    }

    if (session.get())
    {
        session->getLongCallContext().runPostProcessActions();
    }
    smsc_log_debug(log_, "%s: processed", where );
}
 */


/*
void StateMachine::processDataSm(std::auto_ptr<SmppCommand> aucmd, util::HRTiming* inhrt )
{
    const char* where = "DataSm";
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;

    util::HRTiming hrt( inhrt );
    hrt.comment(where);

    smsc_log_debug(log_, "%s: got cmd=%p cmd->serial=%u cmd->sess=%p%s",
                   where,
                   cmd,
                   cmd->getSerial(),
                   cmd->getSession(),
                   cmd->getOperationId() != invalidOpId() ? ", continued..." : "" );
    uint32_t rcnt = 0, failed = 0;
    SmppEntity *src = NULL;
    SmppEntity *dst = NULL;
    ActiveSession session;
    router::RouteInfo ri;
    FixedLengthString<smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH> routeId;
    re::RuleStatus st;
    st.status = re::STATUS_REDIRECT; // to start the loop
    SMS& sms=*(cmd->get_sms());
    SmsCommand& smscmd=cmd->get_smsCommand();
    SessionManager& sm = SessionManager::Instance();
    src = cmd->getEntity();
    smscmd.set_orgDialogId(cmd->get_dialogId());

    if ( sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) )
    {
        smsc_log_info( log_, "%s: USSD dialog not allowed in DataSm %s(%s)->%s",
                       where, sms.getOriginatingAddress().toString().c_str(),
                       src->getSystemId(), 
                       sms.getDestinationAddress().toString().c_str());
        st.status = re::STATUS_FAILED;
        st.result = smsc::system::Status::USSDDLGREFMISM;
    } else {
        smscmd.orgSrc=sms.getOriginatingAddress();
        smscmd.orgDst=sms.getDestinationAddress();
    }

    bool routeset = smscmd.hasRouteSet();
    int statevent = stat::events::smpp::GW_REJECTED;

    while ( st.status == re::STATUS_REDIRECT ) {

        st.status = re::STATUS_OK;
        st.result = 0;

        if ( ! routeset ) {

            dst=routeMan_->RouteSms(src->getSystemId(),sms.getOriginatingAddress(),sms.getDestinationAddress(),ri);
            smsc_log_debug(log_, "%s: orig_route_id=%s, new_route_id=%s",
                           where, routeId.c_str(), ri.routeId.c_str());

            {
                const char* fail = 0;
                if (!dst) {
                    fail = "no route";
                    st.result = smsc::system::Status::NOROUTE;
                } else if ( routeId == ri.routeId ) {
                    fail = "redirect to the same route";
                    st.result = smsc::system::Status::NOROUTE;
                } else if ( dst->getBindType() == btNone ) {
                    fail = "sme not connected";
                    routeId = ri.routeId;
                    st.result = smsc::system::Status::SMENOTCONNECTED;
                    statevent = stat::events::smpp::REJECTED;
                }


                if ( fail ) {
                    smsc_log_info(log_,"%s: %s %s(%s)->%s",
                                  where, 
                                  fail,
                                  sms.getOriginatingAddress().toString().c_str(),
                                  src->getSystemId(),
                                  sms.getDestinationAddress().toString().c_str());
                    st.status = re::STATUS_FAILED;
                    hrt.stop();
                    break;
                }
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

        } else { // route is not set in command
            smscmd.getRouteInfo( ri );
            dst = cmd->getDstEntity();
            routeId = sms.getRouteId();
        }

        hrt.mark( "stm.route" );

        smsc_log_debug( log_, "%s%s: %s. %s(%s)->%s, routeid=%s%s",
                       where,
                       cmd->getOperationId() != invalidOpId() ? " continued..." : "",
                       rcnt ? "(redirected)" : "",
                       sms.getOriginatingAddress().toString().c_str(),
                       src->getSystemId(),
                       sms.getDestinationAddress().toString().c_str(),
                       routeId.c_str(),
                       ri.transit ? "(transit)" : "");

        if ( ri.transit ) {
            hrt.stop();
            break;
        }

        if (!session.get())
        {
            if ( ! routeset ) smscmd.setRouteInfo( ri );
            SessionKey key( (src->info.type == etService) ?
                            sms.getDestinationAddress() : sms.getOriginatingAddress() );
            session = sm.getSession( key, aucmd, ! routeset );
            if ( ! session.get() ) {
                __require__( ! routeset );
                hrt.stop();
                return; // locked
            }
        }

        hrt.mark( "stm.getsess" );

        SmppOperationMaker opmaker( where, aucmd, session, log_ );
        opmaker.process(st, &hrt);
        if ( st.status == re::STATUS_LONG_CALL ) {
            smscmd.setRouteInfo( ri );
            hrt.stop();
            return;
        }

        if ( hrt.mark("stm.rerule") > 100000 ) hrt.comment("/HRTWARN");

        routeset = false;

        if ( rcnt++ >= MAX_REDIRECT_CNT ) {
            smsc_log_info(log_,"%s: noroute(MAX_REDIRECT_CNT reached) %s(%s)->%s",
                          where,
                          sms.getOriginatingAddress().toString().c_str(),
                          src->getSystemId(),
                          sms.getDestinationAddress().toString().c_str());
            st.status = re::STATUS_FAILED;
            st.result = smsc::system::Status::NOROUTE;
            break;
        }

    } // while

    if (st.status != re::STATUS_OK)
    {
        src->putCommand
            ( SmppCommand::makeDataSmResp("0", smscmd.get_orgDialogId(), st.result));
        registerEvent( statevent, src, dst, (char*)ri.routeId, st.result);
        if ( session.get() ) session->closeCurrentOperation();
        hrt.stop();
        return;
    }

    failed = putCommand(DATASM, src, dst, ri, aucmd);

    hrt.mark("stm.putcmd");

    if (session.get()) session->getLongCallContext().runPostProcessActions();

    smsc_log_debug(log_, "%s: processed", where );

    if (failed)
    {
        hrt.comment("/failed");
        std::auto_ptr<SmppCommand> resp(SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),failed));
        resp->setEntity(dst);
        resp->get_resp()->setOrgCmd(aucmd.release());
        resp->setFlag(SmppCommandFlags::FAILED_COMMAND_RESP);
        processSmResp(resp,session);
    }
}
 */


/*
void StateMachine::processDataSmResp(std::auto_ptr<SmppCommand> aucmd, ActiveSession session )
{
    SmppCommand* cmd = aucmd.get();
    if ( ! cmd ) return;
    re::RuleStatus st;
    st.status = re::STATUS_OK;
    st.result = 0;

    static const char* where = "DataSmResp";

    SmppEntity *dst, *src;
    SMS* sms;

    src = cmd->getEntity();

    do { // fake loop

        smsc_log_debug(log_, "%s: got cmd=%p cmd->serial=%u cmd->sess=%p%s",
                       where,
                       cmd,
                       cmd->getSerial(),
                       cmd->getSession(),
                       cmd->getOperationId() != invalidOpId() ? ", continued..." : ""
                       );

        if ( cmd->getOperationId() == invalidOpId() )
        {
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
                        smsc_log_warn(log_, "%s: Src entity disconnected. sid='%s', seq='%d'",
                                      where,
                                      src ? src->getSystemId() : "NULL", cmd->get_dialogId());
                        //          registerEvent(stat::events::smpp::RESP_FAILED, src, NULL, NULL, -1);
                        return;
                    }
                }

                orgCmd = reg_.Get(srcUid, cmd->get_dialogId()).release();
                if( !orgCmd )
                {
                    smsc_log_warn(log_,"%s: Original datasm for datasm response not found. sid='%s',seq='%d'",
                                  where, src->getSystemId(),cmd->get_dialogId());
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
            sms->setOriginatingAddress(smscmd.orgSrc);
            sms->setDestinationAddress(smscmd.orgDst);

            dst = orgCmd->getEntity();
            cmd->setDstEntity(dst);
            cmd->set_dialogId( smscmd.get_orgDialogId() );

            // orgcmd->operation is not set in case of transit route
            if ( orgCmd->getOperationId() == invalidOpId() ) {
                smsc_log_debug(log_, "%s: orgcmd has no operation, transit route?", where );
                break;
            }

        } else {

            // smsc_log_debug(log_, "DataSmResp: continued... cmd=%p", cmd);
            dst = cmd->getDstEntity();
            sms = cmd->get_resp()->getOrgCmd()->get_sms();

        }

        if ( session.get() ) {
            session.moveLock(cmd);
            smsc_log_debug(log_,"%s: got pre-locked session=%p", where, session.get() );

        } else {

            // no session
            SessionKey key( ( cmd->get_resp()->get_dir() == dsdSc2Srv ||
                              cmd->get_resp()->get_dir() == dsdSrv2Srv ) ?
                            sms->getDestinationAddress() : sms->getOriginatingAddress() );
            // NOTE: session should already exist, so if it is not found
            // it means that session has been expired.
            session = SessionManager::Instance().getSession(key, aucmd, false);
            if ( ! aucmd.get() ) {
                // command is taken, session is locked
                return;
            } else if ( ! session.get() ) {
                // session is not found
                smsc_log_warn(log_,"%s: session is not found, expired? key='%s'",
                              where,
                              key.toString().c_str() );
                st.status = re::STATUS_FAILED;
                st.result = smsc::system::Status::TRANSACTIONTIMEDOUT;
                break;
            }


            smsc_log_debug(log_,"%s: got session=%p key='%s' %s",
                           where,
                           session.get(), key.toString().c_str(), cmd->get_resp()->expiredResp ? "(expired)" : "");

        } // if no pre-locked session

        // session should be here

        // create operation
        SmppOperationMaker opmaker( where, aucmd, session, log_ );
        opmaker.process( st );
        if ( st.status == re::STATUS_LONG_CALL ) return;

    } while ( false ); // fake loop

    if (!(cmd->get_resp()->get_messageId()))
    {
        cmd->get_resp()->set_messageId("");
    }

    int staterr;
    int err = int(cmd->get_resp()->getOrigStatus());
    if ( err ) {
        staterr = stat::events::smpp::RESP_REJECTED;
    } else if ( st.result ) {
        cmd->set_status( err = st.result );
        staterr = stat::events::smpp::RESP_GW_REJECTED;
    } else {
        err = cmd->get_status();
        if (cmd->flagSet(SmppCommandFlags::FAILED_COMMAND_RESP))
        {
            staterr = stat::events::smpp::RESP_FAILED;
        }
        else if(cmd->get_resp()->expiredResp)
        {
            staterr = stat::events::smpp::RESP_EXPIRED;
        }
        else
        {
            err = -1;
            staterr = stat::events::smpp::RESP_OK;
        }
    }

    registerEvent( staterr, src, dst, (char*)sms->getRouteId(), err );

    try{
        dst->putCommand(aucmd);
    } catch(std::exception& e) {
        smsc_log_warn(log_,"%s: Failed to put command into %s:%s",
                      where, dst->getSystemId(),e.what());
    }
    
    if (session.get())
    {
        session->getLongCallContext().runPostProcessActions();
        // sessions::SessionManager::Instance().releaseSession(session);
    }

    // cmd->get_resp()->set_sms(0);
    smsc_log_debug(log_, "%s: processed", where );
}
 */


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

            std::auto_ptr< SmppCommand > resp;
            switch( cmd->getCommandId())
            {
            case DELIVERY: {
                resp = SmppCommand::makeDeliverySmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT);
                break;
            }
            case SUBMIT: {
                resp = SmppCommand::makeSubmitSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT, false);
                break;
            }
            case DATASM: {
                resp = SmppCommand::makeDataSmResp("0",cmd->get_dialogId(),smsc::system::Status::DELIVERYTIMEDOUT);
                break;
            }
            default : {
                ::abort();
            }
            } // switch

            resp->get_resp()->expiredResp = true;
            resp->get_resp()->expiredUid = uid;
            resp->setEntity(routeMan_->getSmppEntity(cmd->get_sms()->getDestinationSmeId()));
            resp->get_resp()->setOrgCmd( cmd.release() );
            processSmResp(resp);

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
              throw smsc::util::Exception("MSAG Receipt: Register cmd for uid=%d, seq=%d failed", dst->getUid(), newSeq);
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
