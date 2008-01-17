#pragma ident "$Id$"
/* ************************************************************************* *
 * BillingManager: manages SMSC billing requests on given Connect in
 * asynchronous mode
 * ************************************************************************* */
#ifndef __SMSC_INMAN_BILLING_MANGER_HPP
#define __SMSC_INMAN_BILLING_MANGER_HPP

#include "inman/inman.hpp"
using smsc::inman::AbonentPolicies;
using smsc::inman::INScfCFG;

#include "inman/storage/CDRStorage.hpp"
using smsc::inman::filestore::InBillingFileStorage;

#include "inman/ConnManager.hpp"
using smsc::inman::WorkerAC;
using smsc::inman::ConnectManagerT;

#include "inman/InCacheDefs.hpp"
using smsc::inman::cache::AbonentCacheITF;

#include "inman/common/TaskSchedulerDefs.hpp"
using smsc::util::TaskSchedulerFactoryITF;

namespace smsc    {
namespace inman   {

extern const char * const _CDRmodes[];
extern const char * const _BILLmodes[];
extern const char * const _MSGtypes[];

//Billing modes priority setting: 
// primary billing mode and secondary, that is used in case
// of failure while processing first one
typedef std::pair<ChargeObj::BILL_MODE, ChargeObj::BILL_MODE> BModesPrio;

class BillModes {
protected:
    typedef std::map<ChargeObj::MSG_TYPE, BModesPrio> BModesMAP;
    BModesMAP   bmMap;
    BModesMAP::const_iterator  dflt;
public:
    BillModes()
    {
        std::pair<BModesMAP::iterator, bool> res =
            bmMap.insert(BModesMAP::value_type(ChargeObj::msgUnknown,
                 BModesPrio(ChargeObj::billOFF, ChargeObj::billOFF)));
        dflt = res.first;
    }

    ~BillModes()
    { }

    inline bool assign(ChargeObj::MSG_TYPE msg_type, ChargeObj::BILL_MODE mode_1st,
                        ChargeObj::BILL_MODE mode_2nd = ChargeObj::billOFF)
    {
        std::pair<BModesMAP::iterator, bool> res =
        bmMap.insert(BModesMAP::value_type(msg_type, BModesPrio(mode_1st, mode_2nd)));
        return res.second;
    }

    bool allOFF(void) const
    {
        BModesMAP::const_iterator it = bmMap.begin();
        for (; it != bmMap.end(); ++it) {
            if ((it->second).first != ChargeObj::billOFF)
                return false;
        }
        return true;
    }

    bool useIN(void) const
    {
        BModesMAP::const_iterator it = bmMap.begin();
        for (; it != bmMap.end(); ++it) {
            if ((it->second).first == ChargeObj::bill2IN)
                return true;
        }
        return false;
    }

    bool isAssigned(ChargeObj::MSG_TYPE msg_type) const
    {
        BModesMAP::const_iterator it = bmMap.find(msg_type);
        return (it == bmMap.end()) ? false : true;
    }

    const BModesPrio * modeFor(ChargeObj::MSG_TYPE msg_type) const
    {
        BModesMAP::const_iterator it = bmMap.find(msg_type);
        return (it == bmMap.end()) ? &(dflt->second) : &(it->second);
    }
};


struct BillingCFG {
    enum CDR_MODE { cdrNONE = 0, cdrBILLMODE = 1, cdrALL = 2};
    enum ContractReqMode { reqOnDemand = 0, reqAlways };

    TaskSchedulerFactoryITF * schedMgr;
    AbonentCacheITF *   abCache;
    InBillingFileStorage * bfs;
    TimeWatchersRegistry * twReg;
//billing parameters
    AbonentPolicies * policies;
    BillModes       mo_billMode;
    BillModes       mt_billMode;
    ContractReqMode cntrReq;
    CDR_MODE        cdrMode;
    std::string     cdrDir;         //location to store CDR files
    long            cdrInterval;    //rolling interval for CDR files
    TimeoutHDL      maxTimeout;     //maximum timeout for TCP operations,
                                    //billing aborts on its expiration
    TimeoutHDL      abtTimeout;     //maximum timeout on abonent type requets,
                                    //(HLR & DB interaction), on expiration billing
                                    //continues in CDR mode 
    unsigned short  maxBilling;     //maximum number of Billings per connect

    SS7_CFG         ss7;            //SS7 interaction:
    SmsXServiceMap  smsXMap;        //SMS Extra services iDs and addresses


    BillingCFG() : abCache(NULL), bfs(NULL), twReg(NULL)
        , cdrMode(cdrBILLMODE), cntrReq(reqOnDemand), schedMgr(0)
        , cdrInterval(0), maxBilling(0)
    { }

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

        _cfg.abtTimeout.Init(_cfg.twReg, _cfg.maxBilling);
        _cfg.maxTimeout.Init(_cfg.twReg, _cfg.maxBilling);
    }
    ~BillingManager() { }

    //-- ConnectListenerITF interface
    void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
            /*throw(std::exception)*/;
};

} //inman
} //smsc
#endif /* __SMSC_INMAN_BILLING_MANGER_HPP */

