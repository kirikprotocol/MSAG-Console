#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/TCUniDialogue.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

// {itu-t(0) recommendation(0) q(17) 773 as(1) unidialogue-as(2) version1(1)}
static uint8_t _octs_uniDialogue_as_id[] =
  { 7, 
    0x00,       //itu-t(0) recommendation(0)
    0x11,       //q(17)
    0x86, 0x05, //773
    0x01,       //as(1)
    0x02,       //unidialogue-as(2)
    0x01        //version1(1)
  };

const asn1::EncodedOID _ac_tcap_uniDialogue_as(_octs_uniDialogue_as_id, "_ac_tcap_uniDialogue_as");

} //proto
} //tcap
} //eyeline

