#pragma ident "$Id$"
/* ************************************************************************* *
 * AbntDetectorManager: manages abonent contract requests on given Connect
 * in asynchronous mode.
 * ************************************************************************* */
#ifndef __SMSC_INMAN_ABNT_DETECTOR_MGR_HPP
#define __SMSC_INMAN_ABNT_DETECTOR_MGR_HPP

#include "inman/inman.hpp"
using smsc::inman::AbonentPolicies;
using smsc::inman::INScfCFG;

#include "inman/ConnManager.hpp"
using smsc::inman::WorkerAC;
using smsc::inman::ConnectManagerT;

#include "inman/InCacheDefs.hpp"
using smsc::inman::cache::AbonentCacheITF;

namespace smsc    {
namespace inman   {

struct AbonentDetectorCFG {
    TimeWatchersRegistry * twReg;
    AbonentCacheITF *   abCache;
    AbonentPolicies *   policies;
    TimeoutHDL          abtTimeout;     //maximum timeout on abonent type requests,
                                        //(HLR & DB interaction)
    unsigned short      maxRequests;    //maximum number of requests per connect
    SS7_CFG             ss7;            //SS7 interaction:

    AbonentDetectorCFG() : twReg(0)
    {
        maxRequests = ss7.maxDlgId = ss7.capTimeout = 0;
        ss7.own_ssn = ss7.userId = 0;
    }
};

class AbntDetectorManager: public ConnectManagerT<AbonentDetectorCFG> {
public: 
    AbntDetectorManager(const AbonentDetectorCFG * cfg, unsigned cm_id,
                        Connect* conn, Logger * uselog = NULL)
    : ConnectManagerT<AbonentDetectorCFG>(cfg, cm_id, conn, uselog)
    {
        logger = uselog ? uselog : Logger::getInstance("smsc.inman");
        snprintf(_logId, sizeof(_logId)-1, "AbntMgr[%u]", _cmId);
        //there is only one timeout value AbonentDetectors use.
        _cfg.abtTimeout.Init(_cfg.twReg, _cfg.maxRequests);
    }
    ~AbntDetectorManager()
    { }

    //-- ConnectListenerITF interface
    void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
            /*throw(std::exception)*/;
};

} //inman
} //smsc

#endif /* __SMSC_INMAN_ABNT_DETECTOR_MGR_HPP */

