#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/TCStrDialogue.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

// {itu-t(0) recommendation(0) q(17) 773 as(1) dialogue-as(1) version1(1)}
static uint8_t _octs_strDialogue_as_id[] =
  { 7, 
    0x00,       //itu-t(0) recommendation(0)
    0x11,       //q(17)
    0x86, 0x05, //773
    0x01,       //as(1)
    0x01,       //dialogue-as(1)
    0x01        //version1(1)
  };

const asn1::EncodedOID _ac_tcap_strDialogue_as(_octs_strDialogue_as_id, "_ac_tcap_strDialogue_as");


} //proto
} //tcap
} //eyeline

