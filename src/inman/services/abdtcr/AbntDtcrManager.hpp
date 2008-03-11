#pragma ident "$Id$"
/* ************************************************************************* *
 * AbntDetectorManager: manages abonent contract requests on given Connect
 * in asynchronous mode.
 * ************************************************************************* */
#ifndef __INMAN_ABNT_DETECTOR_MGR_HPP
#define __INMAN_ABNT_DETECTOR_MGR_HPP

#include "inman/incache/AbCacheDefs.hpp"
using smsc::inman::cache::AbonentCacheITF;

#include "inman/services/iapmgr/IAPMgrDefs.hpp"
using smsc::inman::iapmgr::IAPManagerITF;
using smsc::inman::iapmgr::AbonentPolicy;

#include "inman/services/tcpsrv/TCPSrvDefs.hpp"
using smsc::inman::tcpsrv::ConnectManagerT;

#include "inman/services/tmwatch/TimeWatcher.hpp"
using smsc::core::timers::TimeoutHDL;

#include "inman/services/abdtcr/AbntDtcrDefs.hpp"

namespace smsc {
namespace inman {
namespace abdtcr {

struct AbonentDetectorCFG {
    bool                useCache;       //use abonents contract data cache
    AbonentCacheITF *   abCache;
    const AbonentPolicy * iaPol;

    std::string         policyNm;       //name of default AbonenPolicy
    TimeoutHDL          abtTimeout;     //maximum timeout on abonent type requests,
                                        //(Abonentprovider interaction)
    uint16_t            maxRequests;    //maximum number of requests per connect
    uint32_t            cacheTmo;       //abonent cache data expiration timeout in secs

    AbonentDetectorCFG()
        : useCache(false), abCache(0), iaPol(0)
        , abtTimeout(0), maxRequests(0), cacheTmo(0)
    { }
    AbonentDetectorCFG(const AbntDetectorXCFG & use_xcfg)
        : useCache(false), abCache(0), iaPol(0), policyNm(use_xcfg.policyNm)
        , abtTimeout(use_xcfg.abtTimeout), maxRequests(use_xcfg.maxRequests)
        , cacheTmo(use_xcfg.cacheTmo)
    { }
};

class AbntDetectorManager: public ConnectManagerT<AbonentDetectorCFG> {
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

