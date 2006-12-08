#ident "$Id$"
/* ************************************************************************* *
 * BillingManager: manages SMSC billing requests on given Connect in
 * asynchronous mode
 * ************************************************************************* */
#ifndef __SMSC_INMAN_BILLING_MANGER_HPP
#define __SMSC_INMAN_BILLING_MANGER_HPP

#include "inman/inman.hpp"
using smsc::inman::AbonentPolicies;
using smsc::inman::INScfCFG;

#include "inman/common/TimeWatcher.hpp"
using smsc::inman::sync::TimeWatcher;

#include "inman/storage/CDRStorage.hpp"
using smsc::inman::filestore::InBillingFileStorage;

#include "inman/ConnManager.hpp"
using smsc::inman::WorkerAC;
using smsc::inman::ConnectManagerT;

namespace smsc    {
namespace inman   {

typedef enum { BILL_NONE = 0, BILL_ALL, BILL_USSD, BILL_SMS } BILL_MODE;

struct BillingCFG {
    typedef enum { CDR_NONE = 0, CDR_ALL = 1, CDR_POSTPAID = 2} CDR_MODE;
    AbonentCacheITF *   abCache;
    InBillingFileStorage * bfs;
    TimeWatcher *       tmWatcher;
//billing parameters
    AbonentPolicies * policies;
    BILL_MODE       billMode;
    CDR_MODE        cdrMode;
    std::string     cdrDir;         //location to store CDR files
    long            cdrInterval;    //rolling interval for CDR files
    unsigned short  maxTimeout;     //maximum timeout for TCP operations,
                                    //billing aborts on its expiration
    unsigned short  abtTimeout;     //maximum timeout on abonent type requets,
                                    //(HLR & DB interaction), on expiration billing
                                    //continues in CDR mode 
    unsigned short  maxBilling;     //maximum number of Billings per connect

    SS7_CFG         ss7;            //SS7 interaction:
    SmsXServiceMap  smsXMap;        //SMS Extra services iDs and addresses

    BillingCFG()
    {
        abCache = NULL; bfs = NULL; tmWatcher = NULL;
        cdrInterval = 0;
        billMode = smsc::inman::BILL_ALL;
        cdrMode =  CDR_ALL;
        maxTimeout = abtTimeout = maxBilling = ss7.maxDlgId = ss7.capTimeout = 0;
        ss7.own_ssn = ss7.userId = 0;
    }

};

class BillingManager: public ConnectManagerT<BillingCFG> {
public: 
    BillingManager(BillingCFG * cfg, unsigned cm_id, Connect* conn, Logger * uselog = NULL)
        : ConnectManagerT<BillingCFG>(cfg, cm_id, conn, uselog)
    {
        logger = uselog ? uselog : Logger::getInstance("smsc.inman");
        snprintf(_logId, sizeof(_logId)-1, "BillMgr[%u]", _cmId);
    }
    ~BillingManager() { }

    //-- ConnectListenerITF interface
    void onCommandReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
            throw(std::exception);
};

} //inman
} //smsc
#endif /* __SMSC_INMAN_BILLING_MANGER_HPP */

