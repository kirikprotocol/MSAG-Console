#pragma ident "$Id$"
/* ************************************************************************** *
 * TCAP Dispatcher service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_TCAP_DISPATCHER__
#define __INMAN_ICS_TCAP_DISPATCHER__

#include "inman/inap/dispatcher.hpp"

namespace smsc {
namespace inman {
namespace inap {

//TCAP Dispatcher service
class ICSTCDispatcher : public ICServiceAC_T<TCDsp_CFG>,
                        public TCAPDispatcherITF {
protected:
    mutable Mutex   _sync;
    std::auto_ptr<TCDsp_CFG> _cfg;
    std::auto_ptr<TCAPDispatcher> _disp;

    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    inline Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    inline RCode _icsInit(void)
    {
        return _disp->Init(*_cfg.get(), logger) ?
                ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Starts service verifying that all dependent services are started
    inline RCode _icsStart(void)
    {
        return _disp->Start() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
    }
    //Stops service
    inline void  _icsStop(bool do_wait = false)
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
    inline void * Interface(void) const
    {
        return (TCAPDispatcherITF*)_disp.get();
    }


    // -------------------------------------
    // TCAPDispatcherITF interface methods:
    // -------------------------------------
    //Returns state of TCAP unit connection
    inline SS7State_T  ss7State(void) const
    {
        return _disp->ss7State();
    }
    //Binds SSN and initializes SSNSession (TCAP dialogs factory)
    inline SSNSession * openSSN(uint8_t ssn_id, uint16_t max_dlg_id = 2000,
                                uint16_t min_dlg_id = 1, Logger * uselog = NULL)
    {
        return _disp->openSSN(ssn_id, max_dlg_id, min_dlg_id, uselog);
    }
    //
    inline SSNSession* findSession(uint8_t ssn) const
    {
        return _disp->findSession(ssn);
    }
    //
    inline ApplicationContextRegistryITF * acRegistry(void) const
    {
        return _disp->acRegistry();
    }
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_ICS_TCAP_DISPATCHER__ */

