/* ************************************************************************** *
 * Simple console application testing MAP Send Routing Info (Call Handling)
 * service of HLR.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_CHSRI_SERVICE__
#ident "@(#)$Id$"
#define __SMSC_INMAN_CHSRI_SERVICE__

#include <map>

#include "inman/comp/map_chsri/MapCHSRIComps.hpp"
#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"
#include "inman/inap/dispatcher.hpp"

#include "inman/utilbin/MAPUsrCfg.hpp"

namespace smsc  {
namespace inman {

using smsc::util::IMSIString;

using smsc::inman::comp::chsri::CHSendRoutingInfoRes;
using smsc::inman::comp::chsri::CHSendRoutingInfoArg;

using smsc::inman::inap::TCAPDispatcher;
using smsc::inman::inap::MAPUsr_CFG;

using smsc::inman::inap::chsri::CHSRIhandlerITF;
using smsc::inman::inap::chsri::MapCHSRIDlg;

class SRI_CSIListener {
public:
    virtual void onCSIresult(const std::string & subcr_addr, const char * subcr_imsi, const GsmSCFinfo* scfInfo) = 0;
    virtual void onCSIabort(const std::string &subcr_addr, RCHash ercode) = 0;
};

class SRIInterrogator: CHSRIhandlerITF {
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
    // -- CHSRIhandlerITF implementation:
    void onMapResult(CHSendRoutingInfoRes* arg);
    //dialog finalization/error handling:
    //if ercode != 0, no result has been got from MAP service,
    void onEndMapDlg(RCHash ercode = 0);
    //
    inline void Awake(void) { _sync.notify(); }

private:
    mutable EventMonitor   _sync;
    volatile bool   _active;
    TCSessionMA *    tcSesssion;
    MapCHSRIDlg *   sriDlg;
    GsmSCFinfo      scfInfo;
    SRI_CSIListener * csiHdl;
    std::string     subcrAddr;
    IMSIString      subcrImsi;
    Logger *        logger;

    void rlseSRIDialog(void);
};


struct ServiceCHSRI_CFG {
    MAPUsr_CFG      mapCfg;
    SRI_CSIListener * client;
};

class ServiceCHSRI: SRI_CSIListener {
public:
    ServiceCHSRI(const ServiceCHSRI_CFG & in_cfg, Logger * uselog = NULL);
    ~ServiceCHSRI();

    bool start(void);
    void stop(bool do_wait = false);

    //sets subscriber identity: MSISDN addr
    bool requestCSI(const std::string &subcr_addr);

protected:
    friend class SRIInterrogator;
    //-- SRI_CSIListener interface
    void onCSIresult(const std::string &subcr_addr, const char * subcr_imsi, const GsmSCFinfo* scfInfo);
    void onCSIabort(const std::string &subcr_addr, RCHash ercode);

private:
    typedef std::map<std::string, SRIInterrogator *> IntrgtrMAP;
    typedef std::list<SRIInterrogator *> IntrgtrLIST;

    bool getSession(void);
    SRIInterrogator * newWorker(void);

    mutable Mutex   _sync;
    Logger*         logger;
    const char *    _logId;
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

