/* ************************************************************************** *
 * TCAP Dispatcher service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_TCAP_DISPATCHER__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TCAP_DISPATCHER__

#include "inman/inap/dispatcher.hpp"
#include "inman/services/ICSrvDefs.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::ICServiceAC_T;

//TCAP Dispatcher service
class ICSTCDispatcher : public ICServiceAC_T<TCDsp_CFG> {
protected:
    mutable Mutex   _sync;
    std::auto_ptr<TCDsp_CFG> _cfg;
    std::auto_ptr<TCAPDispatcher> _disp;

    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void)
    {
        return _disp->Init(*_cfg.get(), logger) ?
                ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void)
    {
        return _disp->Start() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Stops service
    void  _icsStop(bool do_wait = false)
    {
        _disp->Stop(do_wait);
    }

public:
    ICSTCDispatcher(std::auto_ptr<TCDsp_CFG> & use_cfg,
                    ICServicesHostITF * svc_host, Logger * use_log)
        : ICServiceAC_T<TCDsp_CFG>(
            ICSIdent::icsIdTCAPDisp, svc_host, use_cfg, use_log)
        , _cfg(use_cfg.release()), _disp(new TCAPDispatcher())
    {
        _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSTCDispatcher()
    {
        ICSStop(true);
    }

    //Returns TCAPDispatcherITF
    void * Interface(void) const
    {
        return (TCAPDispatcherITF*)_disp.get();
    }
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_ICS_TCAP_DISPATCHER__ */

