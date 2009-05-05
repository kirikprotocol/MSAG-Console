/* ************************************************************************** *
 * TCAP Dispatcher service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_TCAP_DISPATCHER__
#ident "@(#)$Id$"
#define __INMAN_ICS_TCAP_DISPATCHER__

#include "inman/inap/dispatcher.hpp"

namespace smsc {
namespace inman {
namespace inap {

//TCAP Dispatcher service
class ICSTCDispatcher : public ICServiceAC_T<TCDsp_CFG>
//                        , public TCAPDispatcherITF 
{
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

/*
    // -------------------------------------
    // TCAPDispatcherITF interface methods:
    // -------------------------------------
    //Returns dispatcher state
    DSPState_e  dspState(void) const
    {
        return _disp->dspState();
    }
    //Returns state of TCAP BE unit(s) connection
    SS7State_e  ss7State(void) const
    {
        return _disp->ss7State();
    }
    //Binds SSN and initializes SSNSession (TCAP dialogs factory)
    SSNSession * openSSN(uint8_t ssn_id, uint16_t max_dlg_id = 2000,
                                Logger * uselog = NULL)
    {
        return _disp->openSSN(ssn_id, max_dlg_id, uselog);
    }
    //
    SSNSession* findSession(uint8_t ssn) const
    {
        return _disp->findSession(ssn);
    }
    //
    ApplicationContextRegistryITF * acRegistry(void) const
    {
        return _disp->acRegistry();
    }
*/
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_ICS_TCAP_DISPATCHER__ */

