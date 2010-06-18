#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/proto/ProtocolVersion.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

static const uint8_t _dftltVersionBits[1] = { 0x80 }; //version1

const ProtocolVersion  _dfltProtocolVersion(_dftltVersionBits, 1);

}}}

