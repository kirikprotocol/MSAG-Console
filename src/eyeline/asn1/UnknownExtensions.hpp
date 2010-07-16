/* ************************************************************************** *
 * USSD-Res type definition.
 * ************************************************************************** */
#ifndef __EYELINE_ASN1_UNKNOWN_EXTENSIONS_HPP
#ident "@(#)$Id$"
#define __EYELINE_ASN1_UNKNOWN_EXTENSIONS_HPP

#include <list>
#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {

struct UnknownExtensions {
  std::list<asn1::TransferSyntax> _tsList;
  /* ... */
  /* additional TransferSyntax specific stuff (f.ex. PER bitmask) */
};

}}

#endif /* __EYELINE_MAP_7F0_USSD_RES_HPP */

