/* ************************************************************************** *
 * SCCP constants and definitions.
 * ************************************************************************** */
#ifndef __ELC_SCCP_DEFS_HPP
#ident "@(#)$Id$"
#define __ELC_SCCP_DEFS_HPP

#include <inttypes.h>

namespace eyeline {
namespace sccp {

struct SCCPStandard {
  enum Kind_e { //NOTE: sorted enum !!!
    itut88 = 0  //UDT defined
    , itut93    //XUDT introduced, 'hopCount' field defined
    , itut96    //XUDT enhanced, LUDT introduced, 'importance' field defined
  };
};

//
struct UDTParms {
  enum MsgKind_e { UDT_88 = 0, XUDT_93, XUDT_93_MS, XUDT_96, XUDT_96_MS, LUDT_96 };

  enum Optionals_e { //optional parameters bit masks
    has_IMPORTANCE = 0x01, has_HOPCOUNT = 0x02
  };


  MsgKind_e _kind;
  uint16_t  _segmDataSz;  //maximum user data size per segment
  uint8_t   _numSegm;     //maximum number of segments [1..16] for XUDT, 1 - for others

  UDTParms()
    : _kind(UDT_88), _segmDataSz(0), _numSegm(1)
  { }

  //Calculates the maximum size of user data that may be transferred by
  //most large UDT message defined by specified SCCP standard.
  //Returns 0 if specified optional parameters are incompatible with requested
  //SCCP standard; 
  static uint16_t calculateMaxDataSz(SCCPStandard::Kind_e use_std,
                        uint8_t optionals_mask /* OR-ed Optionals_e values */,
                        uint8_t called_addrLen, uint8_t calling_addrLen);

  //Checks if the specified number of bytes of user data may be transferred by
  //UDT message defined by ITU-T 88 standard (UDT_88).
  //Returns false if user data cann't be fitted into UDT_88 message.
  static bool calculate_itu88(UDTParms & use_parms, 
                  uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz);

  //Checks if the specified number of bytes of user data may be transferred by
  //one of UDT messages defined by ITU-T 93 standard (UDT_88, XUDT_93, XUDT_93_MS).
  //Returns false if user data cann't be fitted into any of available UDT messages.
  static bool calculate_itu93(UDTParms & use_parms, bool use_hopCount,
                  uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz);

  //Checks if the specified number of bytes of user data may be transferred by
  //one of UDT messages defined by ITU-T 96 standard (any of MsgKind_e).
  //Returns false if user data cann't be fitted into any of available UDT messages.
  static bool calculate_itu96(UDTParms & use_parms, 
                  uint8_t optionals_mask /* OR-ed Optionals_e values */,
                  uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz);

  //Calculates type and parameters of UDT message, best suited for transmission
  //of specified number of bytes of user data.
  //Returns false if user data cann't be fitted into any of available UDT messages.
  static bool calculate(UDTParms & use_parms, SCCPStandard::Kind_e use_std,
                        uint8_t optionals_mask /* OR-ed Optionals_e values */,
                        uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz);

  bool calculate(SCCPStandard::Kind_e use_std,
                        uint8_t optionals_mask /* OR-ed Optionals_e values */,
                        uint8_t called_addrLen, uint8_t calling_addrLen, uint16_t data_sz)
  {
    return calculate(*this, use_std, optionals_mask, called_addrLen, calling_addrLen, data_sz);
  }

  uint16_t maxDataSz(void) const { return _segmDataSz * _numSegm; }
};


} //sccp
} //eyeline

#endif /* __ELC_SCCP_DEFS_HPP */

