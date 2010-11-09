/* ************************************************************************** *
 * HLR(SRI) Abonent Provider: implements functionality for quering the HLR 
 * for abonent Camel Subscription Information via SEND_ROUTING_INFO service,
 * determining abonent contract and gsmSCF parameters (address & serviceKey)
 * in case of prepaid type of contract.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPQUERY_HLR_SRI_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAPQUERY_HLR_SRI_HPP

#include "inman/abprov/facility/IAPThrFacility.hpp"
#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri { // (S)end (R)outing (I)nfo

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

using smsc::inman::iaprvd::IAPQueryAC;
using smsc::inman::iaprvd::IAPQueryManagerITF;

using smsc::inman::inap::chsri::MapCHSRIDlg;
using smsc::inman::inap::chsri::CHSRIhandlerITF;


struct IAPQuerySRI_CFG {
    unsigned        mapTimeout;
    TCSessionMA *   mapSess;

    IAPQuerySRI_CFG() { mapTimeout = 20; mapSess = NULL; }
};

class IAPQuerySRI : public IAPQueryAC, CHSRIhandlerITF {
public:
    IAPQuerySRI(unsigned q_id, IAPQueryManagerITF * owner, 
               Logger * use_log, const IAPQuerySRI_CFG & use_cfg);
    // ****************************************
    //-- IAPQueryAC implementation:
    // ****************************************
    int Execute(void);
    void stop(void);
    const char * taskType(void) const { return "IAPQuerySRI"; }

protected:
    friend class smsc::inman::inap::chsri::MapCHSRIDlg;
    // ****************************************
    //-- CHSRIhandlerITF implementation:
    // ****************************************
    void onMapResult(CHSendRoutingInfoRes* arg);
    //dialog finalization/error handling:
    //if ercode != 0, dialog is aborted by reason = errcode
    void onEndMapDlg(RCHash ercode = 0);
    //
    void Awake(void) { _mutex.notify(); }


    IAPQuerySRI_CFG _cfg;
    MapCHSRIDlg *   sriDlg;
};

class IAPQuerySRIFactory : public IAPQueryFactoryITF {
private:
    IAPQuerySRI_CFG  _cfg;
    Logger *        logger;

public:
    IAPQuerySRIFactory(const IAPQuerySRI_CFG &in_cfg, unsigned timeout_secs,
                   Logger * uselog = NULL);
    ~IAPQuerySRIFactory() { }

    // ****************************************
    //-- IAPQueryFactoryITF implementation:
    // ****************************************
    IAPQueryAC * newQuery(unsigned q_id, IAPQueryManagerITF * owner, Logger * use_log);
};

} //sri
} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPQUERY_HLR_SRI_HPP */

