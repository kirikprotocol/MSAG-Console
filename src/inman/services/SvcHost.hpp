/* ************************************************************************* *
 * INMan Configurable Services Host types, interfaces and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_SERVICES_HOST_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_SERVICES_HOST_HPP

#include "inman/services/SvcHostDefs.hpp"
#include "inman/services/SvcHostCfgReader.hpp"

namespace smsc  {
namespace inman {

class SVCHost : public ICServiceAC_T<SvcHostCFG>, 
                public ICServicesHostITF {
private:
    mutable Mutex   _sync;
    const char *    _logId; //logging prefix
    std::auto_ptr<SvcHostCFG> _cfg;

    void initCfg(std::auto_ptr<SvcHostCFG> & use_cfg);

protected:
    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void);
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void);
    //Stops service
    void  _icsStop(bool do_wait = false);

public:
    SVCHost(std::auto_ptr<SvcHostCFG> & use_cfg,
            ICServicesHostITF * svc_host, Logger * use_log = NULL)
        : ICServiceAC_T<SvcHostCFG>(ICSIdent::icsIdSvcHost, NULL, use_cfg, use_log)
        , _logId("SVCHost")
    {
        initCfg(use_cfg);
    }
    ~SVCHost()
    {
        ICSStop(true);
    }

    //Returns ICServicesHostITF
    void * Interface(void) const
    {
        return (ICServicesHostITF*)this;
    }

    // -------------------------------------
    // -- ICServicesHostITF interface mthods
    // -------------------------------------
    ICServiceAC * getICService(ICSUId srv_id) const
    {
        MutexGuard  grd(_sync);
        return _cfg->srvReg.find(srv_id);
    }
    void * getInterface(ICSUId srv_id) const
    {
        MutexGuard  grd(_sync);
        ICServiceAC * pSrv = _cfg->srvReg.find(srv_id);
        return pSrv ? pSrv->Interface() : NULL;
    }
    ICSState getICSrvState(ICSUId srv_id) const
    {
        ICServiceAC * pSrv = getICService(srv_id);
        return pSrv ? pSrv->icsState() : ICServiceAC::icsStIdle;
    }
};


class SVCHostProducer : public 
    ICSProducerXcfAC_T<SVCHost, ICSHostCfgReader, SvcHostCFG> {
public:
    SVCHostProducer()
        : ICSProducerXcfAC_T<SVCHost, ICSHostCfgReader, SvcHostCFG>
            (ICSIdent::icsIdSvcHost)
    { }
    ~SVCHostProducer()
    { }
};

} //inman
} //smsc

#endif /* __INMAN_SERVICES_HOST_HPP */

