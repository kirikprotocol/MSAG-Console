#ifndef __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLECALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLECALLBARREDPARAM_HPP__

#include "eyeline/util/OptionalObjT.hpp"
#include "eyeline/asn1/UnknownExtensions.hpp"

# include "eyeline/map/7_15_0/proto/ERR/CallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {

/*  Type is defined in IMPLICIT tagging environment as follow:
  ExtensibleCallBarredParam ::= SEQUENCE {
        callBarringCause        CallBarringCause        OPTIONAL,
        extensionContainer      ExtensionContainer      OPTIONAL,
        ... ,
        unauthorisedMessageOriginator   [1] NULL                OPTIONAL
} */
struct ExtensibleCallBarredParam {
  util::OptionalObj_T<CallBarringCause> callBarringCause;
  util::OptionalObj_T<ext::ExtensionContainer> extensionContainer;
  util::OptionalObj_T<uint8_t> unauthorisedMessageOriginator;

  asn1::UnknownExtensions   _unkExt;
};

}}}

#endif
