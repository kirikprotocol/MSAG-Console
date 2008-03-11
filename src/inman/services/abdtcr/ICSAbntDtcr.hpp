#pragma ident "$Id$"
/* ************************************************************************** *
 * Abonent contract and gsmSCF parameters determination service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_ABNT_DETECTOR__
#define __INMAN_ICS_ABNT_DETECTOR__

#include "inman/common/ObjRegistryT.hpp"
using smsc::util::POBJRegistry_T;

#include "inman/services/abdtcr/AbntDtcrManager.hpp"
using smsc::inman::ICServiceAC_T;
using smsc::inman::tcpsrv::ConnServiceITF;
using smsc::inman::tcpsrv::ConnectManagerAC;

namespace smsc {
namespace inman {
namespace abdtcr {

//Abonent detector service
class ICSAbntDetector : public ICServiceAC_T<AbntDetectorXCFG>,
                        public ConnServiceITF {
private:
    typedef POBJRegistry_T<uint32_t /*sessId*/,
                            AbntDetectorManager> SessionsRegistry;

    mutable Mutex       _sync;
    const char *        _logId;     //logging prefix
    AbonentDetectorCFG  wCfg;       //configuration for workers
    SessionsRegistry    sesMgrs;    //

protected:
    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    inline Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void);
    //Starts service verifying that all dependent services are started
    inline RCode _icsStart(void) { return ICServiceAC::icsRcOk; }
    //Stops service
    void  _icsStop(bool do_wait = false);

public:
    ICSAbntDetector(std::auto_ptr<AbntDetectorXCFG> & use_cfg,
                    ICServicesHostITF * svc_host, Logger * use_log)
        : ICServiceAC_T<AbntDetectorXCFG>
            (ICSIdent::icsIdAbntDetector, svc_host, use_cfg, use_log)
        , wCfg(*use_cfg.get()), _logId("AbDtcr")
    {
        _icsDeps = use_cfg->icsDeps;
        _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSAbntDetector()
    {
        ICSStop(true);
    }

    //Returns ConnServiceITF
    inline void * Interface(void) const { return (ConnServiceITF*)this; }

    // -------------------------------------
    // ConnServiceITF interface methods:
    // -------------------------------------
    ConnectManagerAC * getConnManager(uint32_t sess_id, Connect * use_conn);
    void               rlseConnManager(uint32_t sess_id);
};

} //abdtcr
} //inman
} //smsc
#endif /* __INMAN_ICS_ABNT_DETECTOR__ */

