#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include <string.h>

#include "eyeline/map/MAPAdrutil.hpp"

namespace eyeline {
namespace map {

using eyeline::sccp::GlobalTitle;

bool convertSCCPAdr2Mobile(const SCCPAddress & sccp_adr, MobileAddress & map_adr)
{
  //verify possibility of ToN <-> NoA conversion
  GlobalTitle::NatureOfAddress_e sccpNoA = sccp_adr.getGT().Indicator().getNoA();
  if ((sccpNoA >= GlobalTitle::noaRsrvNationalMin)
       && (sccpNoA >= GlobalTitle::noaRsrvNationalMax))
    sccpNoA = GlobalTitle::noaNationalReserv;

  switch (sccpNoA) {
  case GlobalTitle::noaUnknown:
    map_adr.setToN(MobileAddress::tonUnknown); break;

  case GlobalTitle::noaSubscriber:
    map_adr.setToN(MobileAddress::tonSubscriber); break;

  case GlobalTitle::noaNationalReserv:
  case GlobalTitle::noaNationalSign:
    map_adr.setToN(MobileAddress::tonNationalSign); break;

  case GlobalTitle::noaInternational:
    map_adr.setToN(MobileAddress::tonInternational); break;

  default:
    return false;
  }

  map_adr.setSignals(sccp_adr.getGT().Signals().c_str());
  //MobileAddress uses the same numeric values of NPi as SCCPAddress does.
  map_adr.setNPi((uint8_t)sccp_adr.getGT().Indicator().getNPi());
  return true;
}

bool convertSCCPAdr2ISDN(const SCCPAddress & sccp_adr, ISDNAddress & isdn_adr)
{
  GlobalTitle::NumberingPlan_e sccpNPi = sccp_adr.getGT().Indicator().getNPi();
  if (sccpNPi != GlobalTitle::npiISDNTele_e164)
    return false;

  return convertSCCPAdr2Mobile(sccp_adr, isdn_adr);
}


} //map
} //eyeline

