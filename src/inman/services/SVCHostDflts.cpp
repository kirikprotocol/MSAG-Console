/* ************************************************************************** *
 * ICServices host default loadUp settings.
 * ************************************************************************** */
#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/services/SVCHostDflts.hpp"

#include "inman/services/tcpsrv/ICSTcpSrvProd.hpp"
#include "inman/services/tmwatch/ICSTMWProd.hpp"
#include "inman/services/abcache/ICSAbCacheProd.hpp"
#include "inman/services/tcapdsp/ICSTCDspProd.hpp"
#include "inman/services/iapmgr/ICSIAPMgrProd.hpp"

namespace smsc  {
namespace inman {
//Default names of sections in XML configuration file
#define _ICS_TCPSRV_CFG_NM      "Host"
#define _ICS_SMBILLING_CFG_NM   "Billing"
#define _ICS_ABNTDTCR_CFG_NM    "AbonentDetector"
#define _ICS_ABNTCACHE_CFG_NM   "AbonentsCache"
#define _ICS_IAPMGR_CFG_NM      "AbonentPolicies"
#define _ICS_TCAPDSP_CFG_NM     "SS7"

/* ************************************************************************** *
 * class ICSLoadupsReg implementation:
 * ************************************************************************** */
ICSLoadupsReg::ICSLoadupsReg() : POBJRegistry_T<ICSUId, ICSLoadupCFG>()
{
    //register loadUps of known services
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdTCPServer,
        new smsc::inman::tcpsrv::ICSProdTcpServer(), _ICS_TCPSRV_CFG_NM));
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdTimeWatcher,
        new smsc::inman::ICSProdTMWatcher(), NULL));
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdAbntCache,
                "libinman_ics_abcache.so", _ICS_ABNTCACHE_CFG_NM));
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdTCAPDisp,
                "libinman_ics_tcapdsp.so", _ICS_TCAPDSP_CFG_NM));
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdIAPManager,
                "libinman_ics_iapmgr.so", _ICS_IAPMGR_CFG_NM));
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdScheduler,
                "libinman_ics_sched.so", NULL));
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdAbntDetector,
                "libinman_ics_abdtcr.so", _ICS_ABNTDTCR_CFG_NM));
    insLoadUp(new ICSLoadupCFG(ICSIdent::icsIdSmBilling,
                "libinman_ics_smbill.so", _ICS_SMBILLING_CFG_NM));

    //Ids of services, which are loaded by default
    dfltIds.insert(ICSIdent::icsIdTCPServer);
    dfltIds.insert(ICSIdent::icsIdTimeWatcher);
}

} // namespace inman
} // namespace smsc

