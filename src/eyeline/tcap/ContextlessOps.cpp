#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/ContextlessOps.hpp"

namespace eyeline {
namespace tcap {

static uint8_t _octs_ac_contextless_ops[] =
//  { itu-t(0) identified-organization(4) etsi(0) mobileDomain(0) gsm-Network(1)
//    ac-Id(0) reserved(127) version127(127) }
    {7, 0x04, 0x00, 0x00, 0x01, 0x00, 0x7F, 0x7F};


EncodedOID _ac_contextless_ops(_octs_ac_contextless_ops, "_ac_contextless_ops");

} //tcap
} //eyeline

