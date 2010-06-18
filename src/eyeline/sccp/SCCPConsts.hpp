/* ************************************************************************** *
 * SCCP Paramenters and constants definition.
 * ************************************************************************** */
#ifndef __ELC_SCCP_PARAM_CONST_DEFS_HPP
#ident "@(#)$Id$"
#define __ELC_SCCP_PARAM_CONST_DEFS_HPP

#include <inttypes.h>

namespace eyeline {
namespace sccp {

struct ReturnCause {
  typedef uint8_t value_type; //8 bits values:
  
  enum ValueId_e {    //according ITU Q.713 clause 3.12
    rcNoTranslationNoA = 0x0, rcNoTranslationAdr = 0x01,
    rcSSNConjestion = 0x02, rcSSNFailure = 0x03, rcUserUneqipped = 0x04,
    rcMTPFailure = 0x05, rcNetworkConjestion = 0x06, rcUnqualified = 0x07,
    rcMsgTransport = 0x08, rcLocalProcessing = 0x09, rcDstReassembly = 0x0A,
    rcSCCPFailure = 0x0B, rcHOPCounter = 0x0C, rcSegmNotSupported = 0x0D,
    rcSegmFailure = 0x0E, 
    rcRsrvInternational = 0x0F, //up to 0xE4
    rcRsrvNational = 0xE5,      //up to 0xFE
    rcReserved = 0xFF
  };

  static ValueId_e value2Id(value_type use_val)
  {
    if (use_val == rcReserved)
      return rcReserved;
    if (use_val >= rcRsrvNational)
      return rcRsrvNational;
    if (use_val >= rcRsrvInternational)
      return rcRsrvInternational;
    return static_cast<ValueId_e>(use_val);
  }
};

typedef ReturnCause::ValueId_e  ReturnCause_e;
typedef ReturnCause::value_type ReturnCause_t;


} //sccp
} //eyeline

#endif /* __ELC_SCCP_PARAM_CONST_DEFS_HPP */

