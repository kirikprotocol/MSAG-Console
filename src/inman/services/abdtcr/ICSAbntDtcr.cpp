#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/ICSAbntDtcr.hpp"
using smsc::inman::iapmgr::IAPManagerITF;
using smsc::core::timers::TimeWatchersRegistryITF;
using smsc::inman::tcpsrv::TCPServerITF;

#include "inman/interaction/msgdtcr/MsgContract.hpp"
using smsc::inman::interaction::INPCSAbntContract;

namespace smsc {
namespace inman {
namespace abdtcr {
/* ************************************************************************** *
 * class ICSAbntDetector implementation.
 * ************************************************************************** */
void ICSAbntDetector::_icsStop(bool do_wait/* = true*/)
{
    if (do_wait) {
        //abort all sessions
        SessionsRegistry::const_iterator it = sesMgrs.begin();
        for (; it != sesMgrs.end(); ++it)
            it->second->Abort();
    }
}

ICServiceAC::RCode ICSAbntDetector::_icsInit(void)
{
    wCfg.iapMgr = (const IAPManagerITF *)
                  _icsHost->getInterface(ICSIdent::icsIdIAPManager);
    //check that default policy is configured
    const AbonentPolicy * dfltPol = wCfg.iapMgr->getPolicy(wCfg.policyNm);
    if (!dfltPol) {
        smsc_log_fatal(logger, "%s: IAPolicy %s is not configured!", _logId,
                       wCfg.policyNm.c_str());
        return ICServiceAC::icsRcError;
    }
    if (wCfg.useCache)
        wCfg.abCache = (AbonentCacheITF*)_icsHost->getInterface(ICSIdent::icsIdAbntCache);
    //there is only one timeout value AbonentDetectors use.
    TimeWatchersRegistryITF * icsTW = (TimeWatchersRegistryITF *)
                                    _icsHost->getInterface(ICSIdent::icsIdTimeWatcher);
    wCfg.abtTimeout.Init(icsTW, wCfg.maxRequests);

    TCPServerITF * tcpSrv = (TCPServerITF *)_icsHost->getInterface(ICSIdent::icsIdTCPServer);
    tcpSrv->registerProtocol(INPCSAbntContract::getInstance(), this);
    return ICServiceAC::icsRcOk;
}

// -------------------------------------
// ConnServiceITF interface methods:
// -------------------------------------
ConnectManagerAC * ICSAbntDetector::getConnManager(uint32_t sess_id, Connect * use_conn)
{
    MutexGuard  grd(_sync);
    if (_icsState != ICServiceAC::icsStStarted)
        return NULL;

    AbntDetectorManager * pMgr = sesMgrs.find(sess_id);
    if (!pMgr) { //create new connect manager
        std::auto_ptr<AbntDetectorManager> mgr
                (new AbntDetectorManager(wCfg, sess_id, use_conn, logger));
        sesMgrs.insert(sess_id, mgr.get());
        return mgr.release();
    }
    //rebind existing one
    pMgr->Bind(use_conn);
    return pMgr;
}

void ICSAbntDetector::rlseConnManager(uint32_t sess_id)
{
    MutexGuard  grd(_sync);
    if (!sesMgrs.erase(sess_id)) {
        smsc_log_error(logger, "%s: SmBillManager[%u] is "
                               "unknown/already released",
                                _logId, sess_id);
    }
}

} //abdtcr
} //inman
} //smsc

