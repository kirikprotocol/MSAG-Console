/* ************************************************************************** *
 * Private AC oid for dealing with contextless operations.
 * ************************************************************************** */
#ifndef __ELC_TCAP_AC_CONTEXTLESS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_AC_CONTEXTLESS_HPP

#include "eyeline/asn1/EncodedOID.hpp"

namespace eyeline {
namespace tcap {

using eyeline::asn1::EncodedOID;

//Private AC oid for dealing with contextless operations (usually MAP v1)
extern EncodedOID _ac_contextless_ops;

} //tcap
} //eyeline

#endif /* __ELC_TCAP_AC_CONTEXTLESS_HPP */

