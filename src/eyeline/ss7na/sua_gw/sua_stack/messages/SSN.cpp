#include "SSN.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

const char*
SSN::getDescription(ssn_t ssn)
{
  if ( ssn == SSN_NOT_KNOWN )
    return "SSN not known/not used";
  if ( ssn == SSN_SCCP_MANAGEMENT )
    return "SCCP management";
  if ( ssn == SSN_ISDN_USER_PART )
    return "ISDN user part";
  if ( ssn == SSN_OMAP )
    return "operation, maintenance and administration part (OMAP)";
  if ( ssn == SSN_MAP )
    return "mobile application part (MAP)";
  if ( ssn == SSN_HLR )
    return "home location register (HLR)";
  if ( ssn == SSN_VLR )
    return "visitor location register (VLR)";
  if ( ssn == SSN_MSC )
    return "mobile switching centre (MSC)";
  if ( ssn == SSN_EIC )
    return "equipment identifier centre (EIC)";
  if ( ssn == SSN_AUC )
    return "authentication centre (AUC)";
  if ( ssn == SSN_ISDN_SUP_SERVICE )
    return "ISDN supplementary services";
  if ( ssn == SSN_BROADBAND_ISDN )
    return "broadband ISDN edge-to-edge applications";
  if ( ssn == SSN_TC_TEST_RESPOPNDER )
    return "TC test responder";
  else
    return "Reserved";
}

}}}}}
