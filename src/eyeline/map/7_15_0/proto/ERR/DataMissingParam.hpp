#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DATAMISSINGPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_DATAMISSINGPARAM_HPP__

# include "util/OptionalObjT.hpp"
# include "eyeline/asn1/UnknownExtensions.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {

using smsc::util::OptionalObj_T;

/* Type is defined in IMPLICIT tagging environment as follow:
 DataMissingParam ::= SEQUENCE {
   extensionContainer      ExtensionContainer      OPTIONAL,
   ...
 } */
struct DataMissingParam {
  OptionalObj_T<ext::ExtensionContainer>  extensionContainer;
  asn1::UnknownExtensions                 _unkExt;
};

}}}

#endif /* __EYELINE_MAP_7F0_PROTO_ERR_DATAMISSINGPARAM_HPP__ */

