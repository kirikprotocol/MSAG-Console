#ifndef __EYELINE_MAP_7F0_PROTO_EXT_PRIVATEEXTENSION_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_EXT_PRIVATEEXTENSION_HPP__

# include "eyeline/util/OptionalObjT.hpp"

# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace map {
namespace ext {

using eyeline::util::OptionalObj_T;

/* Type is defined in IMPLICIT tagging environment as follow:
  PrivateExtension ::= SEQUENCE {
    extId       MAP-EXTENSION.&extensionId ({ExtensionSet}),
    extType     MAP-EXTENSION.&ExtensionType ({ExtensionSet}{@extId}) OPTIONAL
  }
*/

struct PrivateExtension {
  asn1::EncodedOID                    extId;
  OptionalObj_T<asn1::TransferSyntax> extType;
};

}}}

#endif /* __EYELINE_MAP_7F0_PROTO_EXT_PRIVATEEXTENSION_HPP__ */

