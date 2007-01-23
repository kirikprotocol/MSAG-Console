#ident "$Id$"
#ifndef __SMSC_INMAN_CHSRI_SERVICE__
#define __SMSC_INMAN_CHSRI_SERVICE__

#include <map>

#include "inman/comp/acdefs.hpp"
using smsc::ac::ACOID;

#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;

#include "inman/comp/map_chsri/MapCHSRIComps.hpp"
using smsc::inman::comp::chsri::CHSendRoutingInfoRes;
using smsc::inman::comp::chsri::CHSendRoutingInfoArg;

#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"
using smsc::inman::inap::chsri::CHSRIhandler;
using smsc::inman::inap::chsri::MapCHSRIDlg;

namespace smsc  {
namespace inman {

class SRI_CSIListener {
public:
    virtual void onCSIresult(const std::string & subcr_addr, const char * subcr_imsi, const GsmSCFinfo* scfInfo) = 0;
    virtual void onCSIabort(const std::string &subcr_addr, unsigned short ercode, InmanErrorType errLayer) = 0;
};

class SRIInterrogator: CHSRIhandler {
public:
    SRIInterrogator(TCSessionMA* pSession, SRI_CSIListener * csi_listener,
                    Logger * uselog = NULL);
    ~SRIInterrogator();

    bool isActive(void);
    //sets subscriber identity: MSISDN addr
    bool interrogate(const std::string &subcr_addr);
    void cancel(void);

protected:
    friend class smsc::inman::inap::chsri::MapCHSRIDlg;
    //CHSRIhandler interface
    void onMapResult(CHSendRoutingInfoRes* arg);
    //dialog finalization/error handling:
    void onEndMapDlg(unsigned short ercode, InmanErrorType errLayer);

private:
    Mutex           _sync;
    volatile bool   _active;
    TCSessionMA *    tcSesssion;
    MapCHSRIDlg *   sriDlg;
    GsmSCFinfo      scfInfo;
    SRI_CSIListener * csiHdl;
    std::string     subcrAddr;
    char            subcrImsi[MAP_MAX_IMSI_AddressValueLength + 1];
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

struct ServiceCHSRI_CFG {
    SS7_CFG         hlr;
    SRI_CSIListener * client;
};

class ServiceCHSRI: /*public */SRI_CSIListener {
public:
    ServiceCHSRI(const ServiceCHSRI_CFG * in_cfg, Logger * uselog = NULL);
    virtual ~ServiceCHSRI();

    bool start();
    void stop();

    //sets subscriber identity: MSISDN addr
    bool requestCSI(const std::string &subcr_addr);

protected:
    friend class SRIInterrogator;
    //-- SRI_CSIListener interface
    void onCSIresult(const std::string &subcr_addr, const char * subcr_imsi, const GsmSCFinfo* scfInfo);
    void onCSIabort(const std::string &subcr_addr, unsigned short ercode, InmanErrorType errLayer);

private:
    typedef std::map<std::string, SRIInterrogator *> IntrgtrMAP;
    typedef std::list<SRIInterrogator *> IntrgtrLIST;

    SRIInterrogator * newWorker(void);

    Mutex           _sync;
    Logger*         logger;
    SSNSession*     session;
    TCSessionMA *   mapSess;
    TCAPDispatcher* disp;
    volatile bool   running;
    ServiceCHSRI_CFG _cfg;
    IntrgtrMAP      workers;
    IntrgtrLIST     pool;
};


} //inman
} //smsc

#endif /* __SMSC_INMAN_CHSRI_SERVICE__ */

