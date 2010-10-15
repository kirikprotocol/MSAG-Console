#ifndef __EYELINE_MAP_7F0_PROTO_ERR_CALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_CALLBARREDPARAM_HPP__

# include "eyeline/map/7_15_0/proto/ERR/CallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"

# include "eyeline/util/ChoiceOfT.hpp"

namespace eyeline {
namespace map {
namespace err {

/*
 CallBarredParam ::= CHOICE {
   callBarringCause        CallBarringCause,
      -- call BarringCause must not be used in version 3 and higher
   extensibleCallBarredParam       ExtensibleCallBarredParam
      -- extensibleCallBarredParam must not be used in version <3
 }
 */
class CallBarredParam :
  public util::ChoiceOf2_T<CallBarringCause, ExtensibleCallBarredParam> {

public:
  CallBarredParam()
    : util::ChoiceOf2_T<CallBarringCause, ExtensibleCallBarredParam>()
  { }
  ~CallBarredParam()
  { }

  Alternative_T<CallBarringCause>           callBarringCause() { return alternative0(); }
  Alternative_T<ExtensibleCallBarredParam>  extensibleCallBarredParam() { return alternative1(); }

  ConstAlternative_T<CallBarringCause>            callBarringCause() const { return alternative0(); }
  ConstAlternative_T<ExtensibleCallBarredParam>   extensibleCallBarredParam() const { return alternative1(); }
};

}}}

#endif
