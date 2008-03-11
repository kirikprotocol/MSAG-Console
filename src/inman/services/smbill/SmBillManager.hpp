#pragma ident "$Id$"
/* ************************************************************************* *
 * BillingManager: manages SM/USSD messages billing requests on given
 * Connect in asynchronous mode.
 * ************************************************************************* */
#ifndef __INMAN_BILLING_MANAGER_HPP
#define __INMAN_BILLING_MANAGER_HPP

#include "inman/storage/CDRStorage.hpp"
using smsc::inman::filestore::InBillingFileStorage;

#include "inman/incache/AbCacheDefs.hpp"
using smsc::inman::cache::AbonentCacheITF;

#include "inman/services/iapmgr/IAPMgrDefs.hpp"
using smsc::inman::iapmgr::IAPManagerITF;
using smsc::inman::iapmgr::AbonentPolicy;

#include "inman/services/tcpsrv/TCPSrvDefs.hpp"
using smsc::inman::tcpsrv::ConnectManagerT;

#include "inman/services/tmwatch/TimeWatcher.hpp"
using smsc::core::timers::TimeoutHDL;

#include "inman/services/scheduler/TaskSchedulerDefs.hpp"
using smsc::util::TaskSchedulerFactoryITF;

#include "inman/services/smbill/SmBillDefs.hpp"

#include "inman/inap/TCDspDefs.hpp"
using smsc::inman::inap::TCAPDispatcherITF;

namespace smsc   {
namespace inman  {
namespace smbill {

struct SmBillingCFG {
    std::auto_ptr<SmBillParams> prm; //core SM billing parameters
    TimeoutHDL      maxTimeout;     //maximum timeout for TCP operations,
                                    //billing aborts on its expiration
    TimeoutHDL      abtTimeout;     //maximum timeout on abonent type requets,
                                    //(HLR & DB interaction), on expiration billing
                                    //continues in CDR mode 
    const AbonentPolicy *   iaPol;  // 
    AbonentCacheITF *       abCache;
    TCAPDispatcherITF *     tcDisp;
    TaskSchedulerFactoryITF * schedMgr;
    std::auto_ptr<InBillingFileStorage> bfs; //CDR files rolling storage

    SmBillingCFG() : prm(new SmBillParams())
        , iaPol(NULL), abCache(NULL), schedMgr(NULL), tcDisp(NULL)
    { }

    SmBillingCFG(SmBillingXCFG & use_xcfg)
        : prm(use_xcfg.prm.release())
        , maxTimeout(use_xcfg.maxTimeout), abtTimeout(use_xcfg.abtTimeout)
        , iaPol(NULL), abCache(NULL), schedMgr(NULL), tcDisp(NULL)
    { }
};

class SmBillManager: public ConnectManagerT<SmBillingCFG> {
public: 
    SmBillManager(const SmBillingCFG & cfg, unsigned cm_id,
                            Connect* conn, Logger * uselog = NULL)
        : ConnectManagerT<SmBillingCFG>(cfg, cm_id, conn, uselog)
    {
        logger = uselog ? uselog : Logger::getInstance("smsc.inman");
        snprintf(_logId, sizeof(_logId)-1, "BillMgr[%u]", _cmId);
    }
    ~SmBillManager()
    { }

    //-- ConnectListenerITF interface
    void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
            /*throw(std::exception)*/;
};

} //smbill
} //inman
} //smsc
#endif /* __INMAN_BILLING_MANAGER_HPP */

