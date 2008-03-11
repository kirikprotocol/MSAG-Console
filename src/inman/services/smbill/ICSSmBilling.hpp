#pragma ident "$Id$"
/* ************************************************************************** *
 * SMS/USSD messages billing service.
 * ************************************************************************** */
#ifndef __INMAN_ICS_SMBILLING_HPP
#define __INMAN_ICS_SMBILLING_HPP

#include "inman/services/smbill/SmBillManager.hpp"
using smsc::inman::ICServiceAC_T;
using smsc::inman::tcpsrv::ConnServiceITF;
using smsc::inman::tcpsrv::ConnectManagerAC;
using smsc::inman::filestore::InFileStorageRoller;

namespace smsc {
namespace inman {
namespace smbill {

//SMS/USSD messages billing service.
class ICSSmBilling : public ICServiceAC_T<SmBillingXCFG>,
                        public ConnServiceITF {
private:
    typedef POBJRegistry_T<uint32_t /*sessId*/,
                            SmBillManager> SessionsRegistry;

    mutable Mutex       _sync;
    const char *        _logId;     //logging prefix
    SmBillingCFG        wCfg;       //configuration for workers
    SessionsRegistry    sesMgrs;    //
    std::auto_ptr<InFileStorageRoller> roller;

protected:
    // ---------------------------------
    // -- ICServiceAC interface methods
    // --------------------------------- 
    inline Mutex & _icsSync(void) const { return _sync; }
    //Initializes service verifying that all dependent services are inited
    RCode _icsInit(void);
    //Starts service verifying that all dependent services are started
    RCode _icsStart(void);
    //Stops service
    void  _icsStop(bool do_wait = false);

public:
    ICSSmBilling(std::auto_ptr<SmBillingXCFG> & use_cfg,
                    ICServicesHostITF * svc_host, Logger * use_log)
        : ICServiceAC_T<SmBillingXCFG>
            (ICSIdent::icsIdSmBilling, svc_host, use_cfg, use_log)
        , wCfg(*(use_cfg.get())), _logId("SmBill")
    {
        _icsDeps = use_cfg->deps;
        delete use_cfg.release();
        _icsState = ICServiceAC::icsStConfig;
    }
    ~ICSSmBilling()
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

} //smbill
} //inman
} //smsc
#endif /* __INMAN_ICS_SMBILLING_HPP */

