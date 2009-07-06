/* ************************************************************************** *
 * HLR(SRI) Abonent Provider: implements functionality for quering the HLR 
 * for abonent Camel Subscription Information via SEND_ROUTING_INFO service,
 * determining abonent contract and gsmSCF parameters (address & serviceKey)
 * in case of prepaid type of contract.
 * ************************************************************************** *
 * Expects Provider Config subsection formed as follow:
 *
 *  <section name="Config">
 *      <param name="ownAddress" type="string">ISDN_ADDR</param>  <!-- Provider ISDN address -->
 *      <param name="ownSsn" type="int">OWN_SSN</param>
 *      <param name="maxQueries" type="int">500</param>  <!--  max allowed simultaneous HLR queries -->
 *      <param name="mapTimeout" type="int">20</param>   <!-- timeout on MAP operations, units: seconds, max: 65535, default 20 -->
 *      <param name="msrnTimeout" type="int">300</param>  <!--  msrnExpiration timeout, units: secs -->
 *      <!--  OPTIONAL PARAMETERS -->
 *      <param name="fakeSsn" type="int">FAKE_SSN</param> <!--  ssn to substitute in TCAP dialog org addr -->
 *  </section>
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPROVIDER_HLR_SRI_HPP
#ident "@(#)$Id$"
#define SMSC_INMAN_IAPROVIDER_HLR_SRI_HPP

#include "inman/abprov/IAProvider.hpp"
#include "inman/abprov/facility/IAPThrFacility.hpp"
#include "inman/inap/TCDspDefs.hpp"
#include "inman/inap/map_chsri/DlgMapCHSRI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri { // (S)end (R)outing (I)nfo

using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

using smsc::inman::iaprvd::IAProvider;
using smsc::inman::iaprvd::IAProviderCreatorITF;
using smsc::inman::iaprvd::IAProviderITF;
using smsc::inman::iaprvd::IAPQueryAC;
using smsc::inman::iaprvd::IAPQueryManagerITF;

using smsc::inman::inap::TCAPUsr_CFG;
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
    friend smsc::inman::inap::chsri::MapCHSRIDlg;
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


struct IAPCreatorSRI_CFG {
    unsigned        init_threads;
    IAPQuerySRI_CFG qryCfg;
    TCAPUsr_CFG     sriCfg;

    IAPCreatorSRI_CFG() : init_threads(0)
    { }
};

class IAProviderCreatorSRI: public IAProviderCreatorITF {
protected:
    mutable Mutex           _sync;
    std::auto_ptr<IAProviderThreaded> prvd;
    IAProviderThreadedCFG   prvdCfg;
    IAPCreatorSRI_CFG       cfg;
    ICSIdsSet               icsDeps;
    const ICServicesHostITF * icsHost;
    Logger *                logger;

public:
    IAProviderCreatorSRI(const IAPCreatorSRI_CFG & use_cfg, Logger * use_log = NULL);
    ~IAProviderCreatorSRI();

    // ****************************************
    // -- IAProviderCreatorITF interface
    // ****************************************
    IAProvider::Type    type(void)      const { return IAProvider::iapHLR; }
    IAProvider::Ability ability(void)   const { return IAProvider::abContractSCF; }
    const char *        ident(void)     const { return "iapHLR_SRI"; }
    const ICSIdsSet &   ICSDeps(void) const { return icsDeps; }
    //
    void                logConfig(Logger * use_log = NULL) const;
    //Ensures the provider is properly initialized and returns its interface
    //NOTE: Requires the TCAPDispatcher to be connected !!!
    IAProviderITF *     startProvider(const ICServicesHostITF * use_host);
    void                stopProvider(bool do_wait = false);
};


} //sri
} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPROVIDER_HLR_SRI_HPP */

