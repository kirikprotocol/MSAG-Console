// -------------------------------------------------------------------------------------
// NOTE: all UDT parameteres calculations are done according to ITU-T Q.715 clause 8.3.2
// -------------------------------------------------------------------------------------
#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/sccp/SCCPDefs.hpp"

namespace eyeline {
namespace sccp {

//Calculates the maximum size of user data that may be transferred by
//most large UDT message defined by specified SCCP standard.
uint16_t UDTParms::calculateMaxDataSz(SCCPStandard::Kind_e use_std,
                      uint8_t optionals_mask /* OR-ed Optionals_e values */,
                      uint8_t called_addrLen, uint8_t calling_addrLen)
{
  { //check for compliance of optional parameters 
    SCCPStandard::Kind_e minStd = SCCPStandard::itut88;

    if (optionals_mask & UDTParms::has_IMPORTANCE)
      minStd = SCCPStandard::itut96;
    else if (optionals_mask & UDTParms::has_HOPCOUNT)
      minStd = SCCPStandard::itut93;

    if (minStd > use_std)
      return 0;
  }
  if (use_std == SCCPStandard::itut96) { //max is LUDT_96
    return (uint16_t)4072 + ((optionals_mask & UDTParms::has_IMPORTANCE) ? 0 : 4)
            - called_addrLen - calling_addrLen;
  }
  if (use_std == SCCPStandard::itut93) { //max is XUDT_93_MS*16
    return (uint16_t)16*((260-7) - called_addrLen - calling_addrLen);
  }
  //default: SCCPStandard::itut88, max is UDT_88
  return (uint16_t)262 - called_addrLen - calling_addrLen;
}


//Checks if the specified number of bytes of user data may be transferred by
//UDT message defined by ITU-T 88 standard (UDT_88).
//Returns false if user data cann't be fitted into UDT_88 message.
bool UDTParms::calculate_itu88(UDTParms & use_parms, 
                  uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz)
{
  if ((data_sz + called_addrLen + calling_addrLen) <= 262) {
    use_parms._kind = UDT_88;
    use_parms._numSegm = 1;
    use_parms._segmDataSz = data_sz;
    return true;
  }
  return false;
}

//Checks if the specified number of bytes of user data may be transferred by
//one of UDT messages defined by ITU-T 93 standard (UDT_88, XUDT_93, XUDT_93_MS).
//Returns false if user data cann't be fitted into any of available UDT messages.
bool UDTParms::calculate_itu93(UDTParms & use_parms, bool use_hopCount,
                  uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz)
{
  //If 'hopCount' field isn't set check for ITU-T 88 first
  if (!use_hopCount 
      && calculate_itu88(use_parms, called_addrLen, calling_addrLen, data_sz)) {
    return true;
  }
  //check for XUDT_93
  if ((data_sz + called_addrLen + calling_addrLen) <= 260) {
    use_parms._kind = XUDT_93;
    use_parms._numSegm = 1;
    use_parms._segmDataSz = data_sz;
    return true;
  }
  //check for XUDT_93_MS
  {
    uint16_t segmDataSz = (260-7) - called_addrLen - calling_addrLen;
    if (data_sz <= segmDataSz*16) {
      use_parms._kind = XUDT_93_MS;
      use_parms._numSegm = (uint8_t)(((uint32_t)data_sz<<1) + segmDataSz)/(segmDataSz<<1);
      use_parms._segmDataSz = segmDataSz;
      return true;
    }
  }
  return false;
}

//Checks if the specified number of bytes of user data may be transferred by
//one of UDT messages defined by ITU-T 96 standard (any of UDTParms::MsgKind_e).
//Returns false if user data cann't be fitted into any of available UDT messages.
bool UDTParms::calculate_itu96(UDTParms & use_parms,
                  uint8_t optionals_mask /* OR-ed Optionals_e values */,
                  uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz)
{
  //If 'importance' field isn't set check for ITU-T 88/93 first
  if (!(optionals_mask & UDTParms::has_IMPORTANCE)
      && calculate_itu93(use_parms, (optionals_mask & UDTParms::has_HOPCOUNT),
                                    called_addrLen, calling_addrLen, data_sz)) {
    return true;
  }
  //check for XUDT_96
  if ((data_sz + called_addrLen + calling_addrLen) <= 254) {
    use_parms._kind = XUDT_96;
    use_parms._numSegm = 1;
    use_parms._segmDataSz = data_sz;
    return true;
  }
  //check for XUDT_96_MS
  {
    uint16_t segmDataSz = (254-6) - called_addrLen - calling_addrLen;
    if (data_sz <= segmDataSz*16) {
      use_parms._kind = XUDT_96_MS;
      use_parms._numSegm = (uint8_t)(((uint32_t)data_sz<<1) + segmDataSz)/(segmDataSz<<1);
      use_parms._segmDataSz = segmDataSz;
      return true;
    }
  }
  //check for LUDT_96
  if ((data_sz + called_addrLen + calling_addrLen) <= 
       4072 + ((optionals_mask & UDTParms::has_IMPORTANCE) ? 0 : 4)) {
    use_parms._kind = LUDT_96;
    use_parms._numSegm = 1;
    use_parms._segmDataSz = data_sz;
    return true;
  }
  return false;
}


/* ************************************************************************* *
 * class UDTParms implementation
 * ************************************************************************* */
//Calculates type and parameters of UDT message, required for transmission
//of specified number of bytes of user data.
//Returns false if user data cann't bi fitted in any of UDT messages.
bool UDTParms::calculate(UDTParms & use_parms, SCCPStandard::Kind_e use_std,
                  uint8_t optionals_mask /* OR-ed Optionals_e values */,
                  uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz)
{

  SCCPStandard::Kind_e minStd = SCCPStandard::itut88;

  if (optionals_mask & UDTParms::has_IMPORTANCE)
    minStd = SCCPStandard::itut96;
  else if (optionals_mask & UDTParms::has_HOPCOUNT)
    minStd = SCCPStandard::itut93;

  if (minStd <= use_std) {
    switch (use_std) {
    case SCCPStandard::itut96: {
      if (calculate_itu96(use_parms, optionals_mask,
                          called_addrLen, calling_addrLen, data_sz))
        return true;
    } break;

    case SCCPStandard::itut93: {
      if (calculate_itu93(use_parms, (optionals_mask & UDTParms::has_HOPCOUNT),
                          called_addrLen, calling_addrLen, data_sz))
        return true;
    } break;

    default: // SCCPStandard::itut88:
      if (calculate_itu88(use_parms, called_addrLen, calling_addrLen, data_sz))
        return true;
    } /* eosw */
  }
  return false;
}

} //sccp
} //eyeline

