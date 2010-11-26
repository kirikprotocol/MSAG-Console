#ifndef __EYELINE_MAP_7F0_PROTO_ERR_SYSTEMFAILUREPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_SYSTEMFAILUREPARAM_HPP__

# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleSystemFailureParam.hpp"

# include "eyeline/util/ChoiceOfT.hpp"

namespace eyeline {
namespace map {
namespace err {

/*
  SystemFailureParam ::= CHOICE {
      networkResource NetworkResource,
          -- networkResource must not be used in version 3
      extensibleSystemFailureParam    ExtensibleSystemFailureParam
          -- extensibleSystemFailureParam must not be used in version <3
  }
*/
class SystemFailureParam :
  public util::ChoiceOf2_T<common::NetworkResource, ExtensibleSystemFailureParam> {

public:
  SystemFailureParam()
    : util::ChoiceOf2_T<common::NetworkResource, ExtensibleSystemFailureParam>()
  { }
  ~SystemFailureParam()
  { }

  Alternative_T<common::NetworkResource>        networkResource() { return alternative0(); }
  Alternative_T<ExtensibleSystemFailureParam>   extensibleSystemFailureParam() { return alternative1(); }

  ConstAlternative_T<common::NetworkResource>       networkResource() const { return alternative0(); }
  ConstAlternative_T<ExtensibleSystemFailureParam>  extensibleSystemFailureParam() const { return alternative1(); }
};

}}}

#endif /* __EYELINE_MAP_7F0_PROTO_ERR_SYSTEMFAILUREPARAM_HPP__ */

