#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/smbill/ICSSmBilling.hpp"
using smsc::core::timers::TimeWatchersRegistryITF;
using smsc::inman::tcpsrv::TCPServerITF;
using smsc::inman::iapmgr::IAPManagerITF;

#include "inman/interaction/msgbill/MsgBilling.hpp"
using smsc::inman::interaction::INPCSBilling;

#include "inman/comp/cap_sms/CapSMSFactory.hpp"
using smsc::inman::comp::_ac_cap3_sms;
using smsc::inman::comp::initCAP3SMSComponents;

namespace smsc {
namespace inman {
namespace smbill {
/* ************************************************************************** *
 * class ICSSmBilling implementation.
 * ************************************************************************** */

// -------------------------------------
// ICServiceAC interface methods:
// -------------------------------------
//Initializes service verifying that all dependent services are inited
ICServiceAC::RCode ICSSmBilling::_icsInit(void)
{
    if (wCfg.prm->useCache) {
        wCfg.abCache = (AbonentCacheITF*)_icsHost->getInterface(ICSIdent::icsIdAbntCache);
        if (!wCfg.abCache) {
            smsc_log_warn(logger, "Abonents Cache service required but not loaded!");
            wCfg.prm->useCache = false;
        }
    }

    //Initialize CAP3Sms components factory
    if (wCfg.prm->capSms.get()) {
        wCfg.tcDisp = (TCAPDispatcherITF *)_icsHost->getInterface(ICSIdent::icsIdTCAPDisp);
        if (!wCfg.tcDisp->acRegistry()->getFactory(_ac_cap3_sms)
            && !wCfg.tcDisp->acRegistry()->regFactory(initCAP3SMSComponents)) {
            smsc_log_fatal(logger, "ROS factory registration failed: %s!",
                            _ac_cap3_sms.nick());
            return ICServiceAC::icsRcError;
        }
        wCfg.schedMgr = (TaskSchedulerFactoryITF*)_icsHost->getInterface(ICSIdent::icsIdScheduler);
    }
    //initialize CDR storage
    if (wCfg.prm->cdrMode) {
        wCfg.bfs.reset(new InBillingFileStorage(wCfg.prm->cdrDir, 0, logger));
        int oldfs = wCfg.bfs->RFSOpen(true);
        if (oldfs < 0)
            return ICServiceAC::icsRcError;
        smsc_log_debug(logger, "%s: CDR storage opened%s", _logId,
                       oldfs > 0 ? ", old files rolled": "");
         //initialize external storage roller
        if (wCfg.prm->cdrInterval) {
            roller.reset(new InFileStorageRoller(wCfg.bfs.get(),
                                (unsigned long)wCfg.prm->cdrInterval));
            smsc_log_debug(logger, "%s: CDR storage roller inited", _logId);
        }
    }

    //there are only two timeout values Smbilling uses.
    TimeWatchersRegistryITF * icsTW = (TimeWatchersRegistryITF *)
                                    _icsHost->getInterface(ICSIdent::icsIdTimeWatcher);

    if (!wCfg.prm->policyNm.empty()) {
        const IAPManagerITF * iapMgr = (const IAPManagerITF*)
                            _icsHost->getInterface(ICSIdent::icsIdIAPManager);
        if (!(wCfg.iaPol = iapMgr->getPolicy(wCfg.prm->policyNm))) {
            smsc_log_fatal(logger, "%s: IAPolicy %s is not configured!", _logId,
                           wCfg.prm->policyNm.c_str());
            return ICServiceAC::icsRcError;
        }
        wCfg.abtTimeout.Init(icsTW, wCfg.prm->maxBilling);
    }
    wCfg.maxTimeout.Init(icsTW, wCfg.prm->maxBilling);

    TCPServerITF * tcpSrv = (TCPServerITF *)_icsHost->getInterface(ICSIdent::icsIdTCPServer);
    tcpSrv->registerProtocol(INPCSBilling::getInstance(), this);

    return ICServiceAC::icsRcOk;
}

ICServiceAC::RCode ICSSmBilling::_icsStart(void)
{
    if (roller.get())
        roller->Start();
    if (wCfg.abtTimeout.Value() && !wCfg.abtTimeout.Start()) {
        smsc_log_fatal(logger, "%s: TimeWatcher[abt] startup failure!", _logId);
        return ICServiceAC::icsRcError;
    }
    if (!wCfg.maxTimeout.Start()) {
        smsc_log_fatal(logger, "%s: TimeWatcher[max] startup failure!", _logId);
        return ICServiceAC::icsRcError;
    }
    return ICServiceAC::icsRcOk;
}

//Stops service
void  ICSSmBilling::_icsStop(bool do_wait/* = false*/)
{
    if (roller.get())
        roller->Stop(do_wait);
    if (do_wait) {
        //abort all sessions
        SessionsRegistry::const_iterator it = sesMgrs.begin();
        for (; it != sesMgrs.end(); ++it)
            it->second->Abort();
    }
}

// -------------------------------------
// ConnServiceITF interface methods:
// -------------------------------------
ConnectManagerAC * ICSSmBilling::getConnManager(uint32_t sess_id, Connect * use_conn)
{
    MutexGuard  grd(_sync);
    if (_icsState != ICServiceAC::icsStStarted) {
        smsc_log_error(logger, "%s: still not started", _logId);
        return NULL;
    }

    SmBillManager * pMgr = sesMgrs.find(sess_id);
    if (!pMgr) { //create new connect manager
        std::auto_ptr<SmBillManager> mgr
                (new SmBillManager(wCfg, sess_id, use_conn, logger));
        sesMgrs.insert(sess_id, mgr.get());
        return mgr.release();
    }
    //rebind existing one
    pMgr->Bind(use_conn);
    return pMgr;
}

void ICSSmBilling::rlseConnManager(uint32_t sess_id)
{
    MutexGuard  grd(_sync);
    if (!sesMgrs.erase(sess_id)) {
        smsc_log_error(logger, "%s: SmBillManager[%u] is "
                                "unknown/already released",
                                _logId, sess_id);
    }
}

} //smbill
} //inman
} //smsc

