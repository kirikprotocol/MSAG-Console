#ifndef __EYELINE_MAP_7F0_PROTO_ERR_UNEXPECTEDDATAPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_UNEXPECTEDDATAPARAM_HPP__

#include "eyeline/util/OptionalObjT.hpp"

# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {

using eyeline::util::OptionalObj_T;

/* Type is defined in IMPLICIT tagging environment as follow:
 UnexpectedDataParam ::= SEQUENCE {
   extensionContainer      ExtensionContainer      OPTIONAL,
   ...
 } */
struct UnexpectedDataParam {
  OptionalObj_T<ExtensionContainer> extensionContainer;
  asn1::UnknownExtensions           _unkExt;
};

}}}

#endif /* __EYELINE_MAP_7F0_PROTO_ERR_UNEXPECTEDDATAPARAM_HPP__ */

