/* ************************************************************************** *
 * Simple console application testing MAP Any Time Subscription Interrogation
 * service of HLR.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_ATIH_SERVICE__
#ident "@(#)$Id$"
#define __SMSC_INMAN_ATIH_SERVICE__

#include <map>

#include "inman/comp/map_atih/MapATSIComps.hpp"
#include "inman/inap/map_atih/DlgMapATSI.hpp"
#include "inman/inap/dispatcher.hpp"

#include "inman/utilbin/MAPUsrCfg.hpp"

namespace smsc  {
namespace inman {

using smsc::inman::comp::atih::RequestedSubscription;
using smsc::inman::comp::atih::ATSIRes;
using smsc::inman::comp::atih::ATSIArg;

using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::MAPUsr_CFG;

using smsc::inman::inap::atih::ATSIhandlerITF;
using smsc::inman::inap::atih::MapATSIDlg;

class ATCSIListener {
public:
    virtual void onCSIresult(const std::string & subcr_addr, const GsmSCFinfo* scfInfo) = 0;
    virtual void onCSIabort(const std::string &subcr_addr, RCHash ercode) = 0;
};

class ATIInterrogator: ATSIhandlerITF {
public:
    ATIInterrogator(TCSessionMA* pSession, ATCSIListener * csi_listener,
                    Logger * uselog = NULL);
    ~ATIInterrogator();

    bool isActive(void);
    //sets subscriber identity: IMSI or MSISDN addr
    bool interrogate(const RequestedSubscription & req_cfg, const std::string &subcr_addr, bool imsi = false);
    void cancel(void);

protected:
    friend class smsc::inman::inap::atih::MapATSIDlg;
    //ATSIhandlerITF interface
    void onATSIResult(ATSIRes* arg);
    //dialog finalization/error handling:
    //if ercode != 0, no result has been got from MAP service,
    void onEndATSI(RCHash ercode = 0);
    //
    inline void Awake(void) { _sync.notify(); }

private:
    mutable EventMonitor  _sync;
    volatile bool   _active;
    TCSessionMA *   tcSesssion;
    MapATSIDlg *    mapDlg;
    GsmSCFinfo      scfInfo;
    ATCSIListener * csiHdl;
    std::string     subcrAddr;
    Logger *        logger;

    void rlseMapDialog(void);
};

struct ServiceATIH_CFG {
    MAPUsr_CFG      mapCfg;
    ATCSIListener * client;
};

class ServiceATIH: ATCSIListener {
public:
    ServiceATIH(const ServiceATIH_CFG & in_cfg, Logger * uselog = NULL);
    virtual ~ServiceATIH();

    bool start();
    void stop(bool do_wait = false);

    RequestedSubscription & getRequestCfg(void) { return _reqCfg; }
    const RequestedSubscription & getRequestCfg(void) const { return _reqCfg; }
    
    //sets subscriber identity: IMSI or MSISDN addr
    bool requestCSI(const std::string &subcr_addr, bool imsi = true);

protected:
    friend class ATIInterrogator;
    //-- ATCSIListener interface
    void onCSIresult(const std::string &subcr_addr, const GsmSCFinfo* scfInfo);
    void onCSIabort(const std::string &subcr_addr, RCHash ercode);

private:
    typedef std::map<std::string, ATIInterrogator *> IntrgtrMAP;
    typedef std::list<ATIInterrogator *> IntrgtrLIST;

    bool getSession(void);
    ATIInterrogator * newWorker(void);

    mutable Mutex   _sync;
    Logger*         logger;
    const char *    _logId;
    TCSessionMA *   mapSess;
    TCAPDispatcher* disp;
    volatile bool   running;
    ServiceATIH_CFG _cfg;
    IntrgtrMAP      workers;
    IntrgtrLIST     pool;
    RequestedSubscription _reqCfg;
};


} //inman
} //smsc

#endif /* __SMSC_INMAN_ATIH_SERVICE__ */

