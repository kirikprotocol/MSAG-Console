#ifndef __EYELINE_MAP_7F0_PROTO_EXT_PCS_EXTENSIONS_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_EXT_PCS_EXTENSIONS_HPP__

# include "eyeline/asn1/UnknownExtensions.hpp"

namespace eyeline {
namespace map {
namespace ext {

/*
 PCS-Extensions ::= SEQUENCE { ... }
 */
struct PCS_Extensions {
  asn1::UnknownExtensions   _unkExt;
};

}}}

#endif
