#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/MAPAdrutil.hpp"

namespace eyeline {
namespace map {

using eyeline::sccp::GlobalTitle;

static bool convertSCCPAdr2Mobile(const GlobalTitle::GTIndicator & use_gti,
                                  const char * use_signals,
                                  MAPAddressStringAC & map_adr)
{
  //verify possibility of ToN <-> NoA conversion
  GlobalTitle::NatureOfAddress_e sccpNoA = use_gti.getNoA();

  switch (sccpNoA) {
  case GlobalTitle::noaUnknown:
    map_adr.setToN(MAPAddressStringAC::tonUnknown); break;

  case GlobalTitle::noaSubscriber:
    map_adr.setToN(MAPAddressStringAC::tonSubscriber); break;

  case GlobalTitle::noaNationalReserv:
  case GlobalTitle::noaNationalSign:
  case GlobalTitle::noaRsrvNationalRange:
    map_adr.setToN(MAPAddressStringAC::tonNationalSign); break;

  case GlobalTitle::noaInternational:
    map_adr.setToN(MAPAddressStringAC::tonInternational); break;

  default:
    return false;
  }

  map_adr.setSignals(use_signals);
  //MobileAddress uses the same numeric values of NPi as SCCPAddress does.
  map_adr.setNPi((uint8_t)use_gti.getNPi());
  return true;
}

bool convertSCCPAdr2Mobile(const SCCPAddress & sccp_adr, MobileAddress & map_adr)
{
  return convertSCCPAdr2Mobile(sccp_adr.getGT().getGTIndicator(), sccp_adr.getGT().getSignals(), map_adr);
}

bool convertSCCPAdr2ISDN(const SCCPAddress & sccp_adr, ISDNAddress & isdn_adr)
{
  const GlobalTitle::GTIndicator & adrGti = sccp_adr.getGT().getGTIndicator();
  return (adrGti.getNPi() != GlobalTitle::npiISDNTele_e164) ? false :
            convertSCCPAdr2Mobile(adrGti, sccp_adr.getGT().getSignals(), isdn_adr);
}


} //map
} //eyeline

