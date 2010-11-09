/* ************************************************************************** *
 * USSD-Res type definition.
 * ************************************************************************** */
#ifndef __EYELINE_ASN1_UNKNOWN_EXTENSIONS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ASN1_UNKNOWN_EXTENSIONS_HPP

#include <list>
#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {

struct UnknownExtensions {
  typedef std::list<asn1::TransferSyntax> ValueList;

  ValueList   _tsList;
  /* ... */
  /* additional TransferSyntax specific stuff (f.ex. PER bitmask) */
};

}}

#endif /* __EYELINE_MAP_7F0_USSD_RES_HPP */

