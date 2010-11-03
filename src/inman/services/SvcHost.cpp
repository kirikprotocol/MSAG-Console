#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
/* ************************************************************************** *
 * class SVCHost implementation:
 * ************************************************************************** */
#include "inman/services/SvcHost.hpp"

namespace smsc  {
namespace inman {
// --------------------------------------------------------------------------
// -- ICServiceAC_T<> interface methods
// --------------------------------------------------------------------------
void SVCHost::initCfg(std::auto_ptr<SvcHostCFG> & use_cfg)
{
    _cfg.reset(use_cfg.release());
    //create configured services
    for (ICSProducersReg::const_iterator it = _cfg->prodReg.begin();
                                        it != _cfg->prodReg.end(); ++it) {
        ICSProducerCFG * prod = it->second;
        ICServiceAC * pSrv = prod->allc->newService(this, logger);
        if (pSrv)
            _cfg->srvReg.insert(it->first, pSrv);
        else
            throw ConfigException("%s: '%s' service allocation failure",
                           _logId, ICSIdent::uid2Name(it->first));
    }
    _icsState = ICServiceAC::icsStConfig;
}

// --------------------------------------------------------------------------
// -- ICServiceAC interface mthods
// --------------------------------------------------------------------------
//Initializes service verifying that all dependent services are inited
ICServiceAC::RCode SVCHost::_icsInit(void)
{
    ICSUId                      srvId = ICSIdent::icsIdUnknown;
    ICServiceAC *               pSrv = NULL;
    ICServicesReg::size_type    attempt = _cfg->srvReg.size() + 1;
    //initialize configured services
    while ((pSrv = _cfg->srvReg.nextWithLesserState(ICServiceAC::icsStInited, srvId))) {
        _sync.Unlock();
        srvId = pSrv->icsUId();
        bool failed = !--attempt;
        if (!failed) {
            try {
                failed = (pSrv->ICSInit() != ICServiceAC::icsRcOk);
            } catch (const std::exception & exc) {
                smsc_log_fatal(logger, "%s: '%s' ICSInit() exception: %s",
                     _logId, ICSIdent::uid2Name(srvId), exc.what());
                failed = true;
            } catch (...) {
                smsc_log_fatal(logger, "%s: '%s' ICSInit() exception: <unknown>",
                     _logId, ICSIdent::uid2Name(srvId));
                failed = true;
            }
        }
        _sync.Lock();
        if (failed) {
            smsc_log_fatal(logger, "%s: '%s' service initialization failure",
                           _logId, ICSIdent::uid2Name(srvId));
            return ICServiceAC::icsRcError;
        }
    }
    return ICServiceAC::icsRcOk;
}
//Starts service verifying that all dependent services are started
ICServiceAC::RCode SVCHost::_icsStart(void)
{
    ICSUId                      srvId = ICSIdent::icsIdUnknown;
    ICServiceAC *               pSrv = NULL;
    ICServicesReg::size_type    attempt = _cfg->srvReg.size() + 1;
    //start configured services
    while ((pSrv = _cfg->srvReg.nextWithLesserState(ICServiceAC::icsStStarted, srvId))) {
        _sync.Unlock();
        srvId = pSrv->icsUId();
        bool failed = !--attempt;
        if (!failed) {
            try {
                failed = (pSrv->ICSStart() != ICServiceAC::icsRcOk);
            } catch (const std::exception & exc) {
                smsc_log_fatal(logger, "%s: '%s' ICSStart() exception: %s",
                     _logId, ICSIdent::uid2Name(srvId), exc.what());
                failed = true;
            } catch (...) {
                smsc_log_fatal(logger, "%s: '%s' ICSStart() exception: <unknown>",
                     _logId, ICSIdent::uid2Name(srvId));
                failed = true;
            }
        }
        _sync.Lock();
        if (failed) {
            smsc_log_fatal(logger, "%s: '%s' service startup failure",
                           _logId, ICSIdent::uid2Name(srvId));
            return ICServiceAC::icsRcError;
        }
    }
    return ICServiceAC::icsRcOk;
}

//Stops service
void SVCHost::_icsStop(bool do_wait/* = false*/)
{
    unsigned i = 0, iMax = do_wait ? 2 : 1;
    //stop configured services(1st run - notification, 2nd - full stop)
    do {
        if (!_cfg->srvReg.empty()) {
            ICServicesReg::const_iterator it = _cfg->srvReg.end();
            do {
                --it;
                if (i || (it->second->icsState() > ICServiceAC::icsStInited)) {
                    _sync.Unlock();
                    try {
                        smsc_log_debug(logger, "%s: %s '%s'", _logId,
                                       i ? "stopping" : "notifying",
                                       ICSIdent::uid2Name(it->first));
                        it->second->ICSStop(i ? true : false);
                    } catch (const std::exception & exc) {
                        smsc_log_fatal(logger, "%s: '%s' ICSStop() exception: %s",
                             _logId, ICSIdent::uid2Name(it->first), exc.what());
                    } catch (...) {
                        smsc_log_fatal(logger, "%s: '%s' ICSStop() exception: <unknown>",
                             _logId, ICSIdent::uid2Name(it->first));
                    }
                    _sync.Lock();
                }
            } while (it != _cfg->srvReg.begin());
        }
    } while ((++i) < iMax);
}

} // namespace inman
} // namespace smsc

