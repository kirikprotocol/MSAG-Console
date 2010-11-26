#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ABSENTSUBSCRIBERPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ABSENTSUBSCRIBERPARAM_HPP__

# include "eyeline/asn1/UnknownExtensions.hpp"
# include "eyeline/util/OptionalObjT.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/ERR/AbsentSubscriberReason.hpp"

namespace eyeline {
namespace map {
namespace err {

/*
AbsentSubscriberParam ::= SEQUENCE {
        extensionContainer      ExtensionContainer      OPTIONAL,
        ...,
        absentSubscriberReason  [0] AbsentSubscriberReason      OPTIONAL
 }
*/
struct AbsentSubscriberParam {
  util::OptionalObj_T<ext::ExtensionContainer> extensionContainer;
  util::OptionalObj_T<AbsentSubscriberReason> absentSubscriberReason;

  asn1::UnknownExtensions   _unkExt;
};

}}}

#endif
