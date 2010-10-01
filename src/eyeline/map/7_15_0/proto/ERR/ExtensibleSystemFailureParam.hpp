#ifndef __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLESYSTEMFAILUREPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLESYSTEMFAILUREPARAM_HPP__

# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/common/AdditionalNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/FailureCauseParam.hpp"
# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"

namespace eyeline {
namespace map {
namespace err {

using eyeline::util::OptionalObj_T;

/* Type is defined in IMPLICIT tagging environment as follow:
  ExtensibleSystemFailureParam ::= SEQUENCE {
        networkResource       NetworkResource   OPTIONAL,
        extensionContainer    ExtensionContainer      OPTIONAL,
        ...,
        additionalNetworkResource   [0] AdditionalNetworkResource   OPTIONAL,
        failureCauseParam           [1] FailureCauseParam   OPTIONAL
} */

struct ExtensibleSystemFailureParam {
  OptionalObj_T<common::NetworkResource>  networkResource;
  OptionalObj_T<ext::ExtensionContainer>  extensionContainer;
  OptionalObj_T<common::AdditionalNetworkResource>  additionalNetworkResource;
  OptionalObj_T<FailureCauseParam>        failureCauseParam;
  asn1::UnknownExtensions                 _unkExt;
};

}}}

#endif /* __EYELINE_MAP_7F0_PROTO_ERR_EXTENSIBLESYSTEMFAILUREPARAM_HPP__ */

