#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ABSENTSUBSCRIBERPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ABSENTSUBSCRIBERPARAM_HPP__

# include "util/OptionalObjT.hpp"
# include "eyeline/asn1/UnknownExtensions.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/ERR/AbsentSubscriberReason.hpp"

namespace eyeline {
namespace map {
namespace err {

using smsc::util::OptionalObj_T;
/*
AbsentSubscriberParam ::= SEQUENCE {
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        absentSubscriberReason  [0] AbsentSubscriberReason      OPTIONAL
 }
*/
struct AbsentSubscriberParam {
  OptionalObj_T<ext::ExtensionContainer> extensionContainer;
  OptionalObj_T<AbsentSubscriberReason> absentSubscriberReason;

  asn1::UnknownExtensions   _unkExt;
};

}}}

#endif
