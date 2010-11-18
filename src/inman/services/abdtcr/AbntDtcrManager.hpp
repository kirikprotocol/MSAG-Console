/* ************************************************************************* *
 * AbntDetectorManager: manages abonent contract requests on given Connect
 * in asynchronous mode.
 * ************************************************************************* */
#ifndef __INMAN_ABNT_DETECTOR_MGR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ABNT_DETECTOR_MGR_HPP

#include "inman/incache/AbCacheDefs.hpp"
#include "inman/services/iapmgr/IAPMgrDefs.hpp"
#include "inman/services/tcpsrv/TCPSrvDefs.hpp"
#include "inman/services/tmwatch/TimeWatcher.hpp"
#include "inman/services/abdtcr/AbntDtcrDefs.hpp"
#include "inman/INManErrors.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

using smsc::inman::INManErrorId;
using smsc::inman::cache::AbonentCacheITF;

using smsc::inman::iapmgr::IAPManagerITF;
using smsc::inman::iapmgr::AbonentPolicy;
using smsc::inman::iapmgr::AbonentPolicyName_t;

using smsc::inman::tcpsrv::ConnectManagerT;
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::SerializablePacketAC;
using smsc::core::timers::TimeoutHDL;

struct AbonentDetectorCFG {
    bool                useCache;       //use abonents contract data cache
    AbonentCacheITF *   abCache;
    const IAPManagerITF * iapMgr;

    AbonentPolicyName_t policyNm;       //name of default AbonenPolicy
    TimeoutHDL          abtTimeout;     //maximum timeout on abonent type requests,
                                        //(Abonentprovider interaction)
    uint16_t            maxRequests;    //maximum number of requests per connect
    uint32_t            cacheTmo;       //abonent cache data expiration timeout in secs

    AbonentDetectorCFG()
        : useCache(false), abCache(0), iapMgr(0)
        , abtTimeout(0), maxRequests(0), cacheTmo(0)
    { }
    AbonentDetectorCFG(const AbntDetectorXCFG & use_xcfg)
        : useCache(use_xcfg.useCache), abCache(0), iapMgr(0), policyNm(use_xcfg.policyNm.c_str())
        , abtTimeout(use_xcfg.abtTimeout), maxRequests(use_xcfg.maxRequests)
        , cacheTmo(use_xcfg.cacheTmo)
    { }
};

class AbntDetectorManager: public ConnectManagerT<AbonentDetectorCFG> {
protected:
    //Composes and sends ContractResult packet
    //returns -1 on error, or number of total bytes sent
    int denyRequest(unsigned dlg_id, INManErrorId::Code_e use_error);

public: 
    AbntDetectorManager(const AbonentDetectorCFG & cfg, uint32_t cm_id,
                        Connect* use_conn, Logger * uselog)
    : ConnectManagerT<AbonentDetectorCFG>(cfg, cm_id, use_conn, uselog)
    {
        logger = uselog ? uselog : Logger::getInstance("smsc.inman");
        snprintf(_logId, sizeof(_logId)-1, "AbntMgr[%u]", _cmId);
    }
    ~AbntDetectorManager()
    { }

    //-- ConnectListenerITF interface
    void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
            /*throw(std::exception)*/;
};

} //abdtcr
} //inman
} //smsc

#endif /* __INMAN_ABNT_DETECTOR_MGR_HPP */

