#ifndef __EYELINE_MAP_7F0_PROTO_EXT_EXTENSIONCONTAINER_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_EXT_EXTENSIONCONTAINER_HPP__

# include "eyeline/map/7_15_0/proto/EXT/PrivateExtensionList.hpp"
# include "eyeline/map/7_15_0/proto/EXT/PCS_Extensions.hpp"

namespace eyeline {
namespace map {
namespace ext {

/* Type is defined in IMPLICIT tagging environment as follow:
  ExtensionContainer ::= SEQUENCE {
    privateExtensionList    [0] PrivateExtensionList OPTIONAL,
    pcs-Extensions          [1] PCS-Extensions       OPTIONAL,
    ...
  }
*/

struct ExtensionContainer {
  OptionalObj_T<PrivateExtensionList> privateExtensionList;
  OptionalObj_T<PCS_Extensions>       pcs_Extensions;
  asn1::UnknownExtensions             _unkExt;
};

}}}

#endif /* __EYELINE_MAP_7F0_PROTO_EXT_EXTENSIONCONTAINER_HPP__ */

