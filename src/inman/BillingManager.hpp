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

extern const char * const _CDRmodes[];
extern const char * const _BILLmodes[];
extern const char * const _MSGtypes[];

class BillModes {
protected:
    typedef std::map<ChargeObj::MSG_TYPE, ChargeObj::BILL_MODE> BModesMAP;
    BModesMAP   bmMap;
    ChargeObj::BILL_MODE    bmBadCfgIN; //default billing mode in case
                                               //of misconfigured IN detected
public:
    BillModes() : bmBadCfgIN(ChargeObj::billOFF)
    { }
    ~BillModes()
    { }

    inline void setBadCfgINMode(ChargeObj::BILL_MODE b_mode) { bmBadCfgIN = b_mode; }

    inline ChargeObj::BILL_MODE  modeForBadCfgIN(void) const { return bmBadCfgIN; }

    inline void assign(ChargeObj::MSG_TYPE msg_type, ChargeObj::BILL_MODE b_mode)
    {
        bmMap.insert(BModesMAP::value_type(msg_type, b_mode));
    }

    bool allOFF(void) const
    {
        BModesMAP::const_iterator it = bmMap.begin();
        for (; it != bmMap.end(); ++it) {
            if (it->second != ChargeObj::billOFF)
                return false;
        }
        return true;
    }

    bool useIN(void) const
    {
        BModesMAP::const_iterator it = bmMap.begin();
        for (; it != bmMap.end(); ++it) {
            if (it->second == ChargeObj::bill2IN)
                return true;
        }
        return false;
    }

    bool isAssigned(ChargeObj::MSG_TYPE msg_type) const
    {
        BModesMAP::const_iterator it = bmMap.find(msg_type);
        return (it == bmMap.end()) ? false : true;
    }

    ChargeObj::BILL_MODE modeFor(ChargeObj::MSG_TYPE msg_type)
    {
        BModesMAP::const_iterator it = bmMap.find(msg_type);
        return (it == bmMap.end()) ? ChargeObj::billOFF : it->second;
    }
};

struct BillingCFG {
    typedef enum { cdrNONE = 0, cdrBILLMODE = 1, cdrALL = 2} CDR_MODE;
    enum ContractReqMode { reqOnDemand = 0, reqAlways };

    AbonentCacheITF *   abCache;
    InBillingFileStorage * bfs;
    TimeWatcher *       tmWatcher;
//billing parameters
    AbonentPolicies * policies;
    BillModes       billMode;
    ContractReqMode cntrReq;
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

    BillingCFG() : abCache(NULL), bfs(NULL), tmWatcher(NULL)
        , cdrMode(cdrBILLMODE), cntrReq(reqOnDemand)
    {
        cdrInterval = maxTimeout = abtTimeout = maxBilling = 0;
    }

    static const char * cdrModeStr(CDR_MODE mode_id)
    {
        return _CDRmodes[mode_id];
    }

    static const char * msgTypeStr(ChargeObj::MSG_TYPE msg_id)
    {
        return _MSGtypes[msg_id];
    }
    static const char * billModeStr(ChargeObj::BILL_MODE mode_id)
    {
        return _BILLmodes[mode_id];
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
    void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
            /*throw(std::exception)*/;
};

} //inman
} //smsc
#endif /* __SMSC_INMAN_BILLING_MANGER_HPP */

