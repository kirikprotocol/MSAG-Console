#ident "$Id$"

#ifndef __SMSC_INMAN_ATIH_SERVICE__
#define __SMSC_INMAN_ATIH_SERVICE__

#include <map>

#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;

#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;

#include "inman/comp/map_atih/MapATSIComps.hpp"
using smsc::inman::comp::atih::ATSIRes;
using smsc::inman::comp::atih::ATSIArg;
using smsc::inman::comp::atih::MAPSCFinfo;

#include "inman/inap/map_atih/DlgMapATSI.hpp"
using smsc::inman::inap::atih::ATSIhandler;
using smsc::inman::inap::atih::MapATSIDlg;

namespace smsc  {
namespace inman {

class ATCSIListener {
public:
    virtual void onCSIresult(const std::string & subcr_addr, const MAPSCFinfo* scfInfo) = 0;
    virtual void onCSIabort(const std::string &subcr_addr, unsigned short ercode, InmanErrorType errLayer) = 0;
};

class ATIInterrogator: ATSIhandler {
public:
    ATIInterrogator(SSNSession* pSession, ATCSIListener * csi_listener,
                    Logger * uselog = NULL);
    ~ATIInterrogator();

    bool isActive(void);
    //sets subscriber identity: IMSI or MSISDN addr
    bool interrogate(const std::string &subcr_addr, bool imsi = false);
    void cancel(void);

protected:
    friend class smsc::inman::inap::atih::MapATSIDlg;
    //ATSIhandler interface
    void onATSIResult(ATSIRes* arg);
    //dialog finalization/error handling:
    void onEndATSI(unsigned short ercode, InmanErrorType errLayer);

private:
    Mutex           _sync;
    volatile bool   _active;
    const char *    ownAdr;
    SSNSession *    tcSesssion;
    MapATSIDlg *    mapDlg;
    MAPSCFinfo      scfInfo;
    ATCSIListener * csiHdl;
    std::string     subcrAddr;
    Logger *        logger;
};


struct SS7_CFG {
//SS7 interaction:
    unsigned char   userId;         //PortSS7 user id [1..20]
    unsigned short  mapTimeout;     //optional timeout for operations with HLR platform
    const char*     scf_addr;       //
    int             scf_ssn;        //
    int             hlr_ssn;        //
};

struct ServiceATIH_CFG {
    SS7_CFG         hlr;
    ATCSIListener * client;
};

class ServiceATIH: /*public */ATCSIListener {
public:
    ServiceATIH(const ServiceATIH_CFG * in_cfg, Logger * uselog = NULL);
    virtual ~ServiceATIH();

    bool start();
    void stop();

    //sets subscriber identity: IMSI or MSISDN addr
    bool requestCSI(const std::string &subcr_addr, bool imsi = true);

protected:
    friend class ATIInterrogator;
    //-- ATCSIListener interface
    void onCSIresult(const std::string &subcr_addr, const MAPSCFinfo* scfInfo);
    void onCSIabort(const std::string &subcr_addr, unsigned short ercode, InmanErrorType errLayer);

private:
    typedef std::map<std::string, ATIInterrogator *> IntrgtrMAP;
    typedef std::list<ATIInterrogator *> IntrgtrLIST;

    ATIInterrogator * newWorker(void);

    Mutex           _sync;
    Logger*         logger;
    SSNSession*     session;
    TCAPDispatcher* disp;
    volatile bool   running;
    ServiceATIH_CFG _cfg;
    IntrgtrMAP      workers;
    IntrgtrLIST     pool;
};


} //inman
} //smsc

#endif /* __SMSC_INMAN_ATIH_SERVICE__ */

