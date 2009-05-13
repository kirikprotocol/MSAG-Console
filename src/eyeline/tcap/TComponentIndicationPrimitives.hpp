#ifndef __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCOMPONENTINDICATIONPRIMITIVES_HPP__

# include "eyeline/tcap/TComponentHandlingPrimitive.hpp"

namespace eyeline {
namespace tcap {

class TC_L_Cancel_Ind : public TComponentHandlingPrimitive {
};

class TC_L_Reject_Ind : public TComponentHandlingPrimitive {
public:
  TC_L_Reject_Ind()
    : _isSetProblemCode(false) {}

  typedef uint8_t problem_code_t;

  void setProblemCode(problem_code_t problemCode) { _problemCode = problemCode; _isSetProblemCode = true; }
  problem_code_t getProblemCode() const {
    if ( _isSetProblemCode )
      return _problemCode;
    else
      throw utilx::FieldNotSetException("TC_L_Reject_Ind::getProblemCode::: problemCode is not set");
  }

private:
  problem_code_t _problemCode;
  bool _isSetProblemCode;
};

}}

#endif
