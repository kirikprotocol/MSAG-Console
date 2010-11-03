/* ************************************************************************** *
 * ICServices registry default loadUp settings.
 * ************************************************************************** */
#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#ifdef ICS_LINKAGE_static
#define ICS_ABNTCACHE_LINKAGE_static
#define EIN_SS7_LINKAGE_static
#define ICS_IAPMGR_LINKAGE_static
#define ICS_SCHED_LINKAGE_static
#define ICS_ABNTDTCR_LINKAGE_static
#define ICS_SMBILLIING_LINKAGE_static
#define ICS_IAPSRI_LINKAGE_static
#endif /* ICS_LINKAGE_static */

#include "inman/services/ICSRegistry.hpp"

#include "inman/services/tcpsrv/ICSTcpSrvLoader.hpp"
#include "inman/services/tmwatch/ICSTMWLoader.hpp"

#include "inman/services/abcache/ICSAbCacheLoader.hpp"
#include "inman/services/tcapdsp/ICSTCDspLoader.hpp"
#include "inman/services/iapmgr/ICSIAPMgrLoader.hpp"
#include "inman/services/iapSRI/ICSIAPrvdSRILoader.hpp"
#include "inman/services/scheduler/ICSSchedLoader.hpp"
#include "inman/services/abdtcr/ICSAbntDtcrLoader.hpp"
#include "inman/services/smbill/ICSSmBillLoader.hpp"


namespace smsc  {
namespace inman {

#ifdef EIN_HD
#define NM_SS7_CFG_SECTION "SS7_HD"
#else /* EIN_HD */
#define NM_SS7_CFG_SECTION "SS7"
#endif /* EIN_HD */

//Default names of sections in XML configuration file
#define _ICS_TCPSRV_CFG_NM      "Host"
#define _ICS_SMBILLING_CFG_NM   "Billing"
#define _ICS_ABNTDTCR_CFG_NM    "AbonentDetector"
#define _ICS_ABNTCACHE_CFG_NM   "AbonentsCache"
#define _ICS_IAPMGR_CFG_NM      "AbonentPolicies"
#define _ICS_TCAPDSP_CFG_NM     NM_SS7_CFG_SECTION

/* ************************************************************************** *
 * class ICSLoadupsReg implementation:
 * ************************************************************************** */
ICSLoadupsReg::ICSLoadupsReg()
{
    //register loadUps of known services
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdTCPServer,
        smsc::inman::ICSLoaderTcpServer, _ICS_TCPSRV_CFG_NM));
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdTimeWatcher,
        smsc::inman::ICSLoaderTMWatcher, NULL));

#if defined(ICS_ABNTCACHE_LINKAGE_static)
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdAbntCache,
                smsc::inman::ICSLoaderAbCache, _ICS_ABNTCACHE_CFG_NM));
#else  /* ICS_ABNTCACHE_LINKAGE_static */
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdAbntCache,
                "libinman_ics_abcache.so", _ICS_ABNTCACHE_CFG_NM));
#endif /* ICS_ABNTCACHE_LINKAGE_static */

#if defined(EIN_HD) || defined(EIN_SS7_LINKAGE_static)
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdTCAPDisp,
                smsc::inman::ICSLoaderTCAPDispatcher, _ICS_TCAPDSP_CFG_NM));
#else  /* EIN_HD || EIN_SS7_LINKAGE_static */
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdTCAPDisp,
                "libinman_ics_tcapdsp.so", _ICS_TCAPDSP_CFG_NM));
#endif /* EIN_HD || EIN_SS7_LINKAGE_static */

#if defined(ICS_IAPMGR_LINKAGE_static)
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdIAPManager,
                smsc::inman::ICSLoaderIAPManager, _ICS_IAPMGR_CFG_NM));
#else  /* ICS_IAPMGR_LINKAGE_static */
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdIAPManager,
                "libinman_ics_iapmgr.so", _ICS_IAPMGR_CFG_NM));
#endif /* ICS_IAPMGR_LINKAGE_static */

#if defined(ICS_SCHED_LINKAGE_static)
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdScheduler,
                smsc::inman::ICSLoaderScheduler, NULL));
#else  /* ICS_SCHED_LINKAGE_static */
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdScheduler,
                "libinman_ics_sched.so", NULL));
#endif /* ICS_SCHED_LINKAGE_static */

#if defined(ICS_ABNTDTCR_LINKAGE_static)
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdAbntDetector,
                smsc::inman::ICSLoaderAbntDetector, _ICS_ABNTDTCR_CFG_NM));
#else  /* ICS_ABNTDTCR_LINKAGE_static */
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdAbntDetector,
                "libinman_ics_abdtcr.so", _ICS_ABNTDTCR_CFG_NM));
#endif /* ICS_ABNTDTCR_LINKAGE_static */

#if defined(ICS_SMBILLIING_LINKAGE_static)
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdSmBilling,
                smsc::inman::ICSLoaderSmBilling, _ICS_SMBILLING_CFG_NM));
#else  /* ICS_SMBILLIING_LINKAGE_static */
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdSmBilling,
                "libinman_ics_smbill.so", _ICS_SMBILLING_CFG_NM));
#endif /* ICS_SMBILLIING_LINKAGE_static */

#if defined(ICS_IAPSRI_LINKAGE_static)
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIAPrvdSRI,
                smsc::inman::ICSLoaderIAPrvdSRI, NULL));
#else  /* ICS_IAPSRI_LINKAGE_static */
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIAPrvdSRI,
                "libinman_iap_sri.so", NULL));
#endif /* ICS_IAPSRI_LINKAGE_static */


    //Ids of services, which are loaded by default
    _dfltIds.insert(ICSIdent::icsIdTCPServer);
    _dfltIds.insert(ICSIdent::icsIdTimeWatcher);
#if defined(EIN_HD) || defined(EIN_SS7_LINKAGE_static)
    _dfltIds.insert(ICSIdent::icsIdTCAPDisp);
#endif /* EIN_HD || EIN_SS7_LINKAGE_static */
}

} // namespace inman
} // namespace smsc

