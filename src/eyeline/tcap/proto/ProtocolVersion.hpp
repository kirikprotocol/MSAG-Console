/* ************************************************************************** *
 * TCAP Dialogue Protocol version type.
 * ************************************************************************** */
#ifndef __TCAP_PROTOCOL_VERSION_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __TCAP_PROTOCOL_VERSION_HPP

#include <inttypes.h>
#include "eyeline/util/BITArray.hpp"

namespace eyeline {
namespace tcap {
namespace proto {

typedef eyeline::util::BITArray_T<uint8_t, 8> ProtocolVersion;

extern const ProtocolVersion  _dfltProtocolVersion; //version1

} //proto
} //tcap
} //eyeline

#endif /* __TCAP_PROTOCOL_VERSION_HPP */

