/* ************************************************************************** *
 * Remote Operations Reject problem according to 
 * joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
  * ************************************************************************** */
#ifndef __ROS_REJECT_PROBLEM_DEFS_HPP
#ident "@(#)$Id$"
#define __ROS_REJECT_PROBLEM_DEFS_HPP

namespace eyeline {
namespace ros {

class RejectProblem {
public:
  enum ProblemKind_e {
    rejGeneral = 0, rejInvoke = 1, rejResult = 2, rejError = 3
  };
  enum GeneralProblem_e {
    rjg_unrecognizedPDU = 0, rjg_mistypedPDU = 1, rjg_badlyStructuredPDU = 2
  };
  enum InvokeProblem_e {    //invocation problems
      rji_duplicateInvocation = 0, rji_unrecognizedOperation = 1
    , rji_mistypedArgument = 2, rji_resourceLimitation = 3
    , rji_releaseInProgress = 4, rji_unrecognizedLinkedId = 5
    , rji_linkedResponseUnexpected = 6, rji_unexpectedLinkedOperation = 7
  };
  enum RResultProblem_e {   //returnResult(NL) problems
      rjr_unrecognizedInvocation = 0, rjr_resultResponseUnexpected = 1
    , rjr_mistypedResult = 2
  };
  enum RErrorProblem_e {    //returnError problems
      rje_unrecognizedInvocation = 0, rje_errorResponseUnexpected =1
    , rje_unrecognizedError = 2, rje_unexpectedError = 3
    , rje_mistypedParameter = 4
  };

protected:
  const ProblemKind_e rejKind;

  union {
    GeneralProblem_e  general;
    InvokeProblem_e   invoke;
    RResultProblem_e  rResult;
    RErrorProblem_e   rError;
  } rejCode;

public:
  RejectProblem(GeneralProblem_e rjg_code = rjg_unrecognizedPDU)
      : rejKind(rejGeneral)
  {
    rejCode.general = rjg_code;
  }
  RejectProblem(InvokeProblem_e rji_code)
      : rejKind(rejInvoke)
  {
    rejCode.invoke = rji_code;
  }
  RejectProblem(RResultProblem_e rjr_code)
      : rejKind(rejResult)
  {
    rejCode.rResult = rjr_code;
  }
  RejectProblem(RErrorProblem_e rje_code)
      : rejKind(rejError)
  {
    rejCode.rError = rje_code;
  }

  ProblemKind_e     problemKind(void) const { return rejKind; }

  GeneralProblem_e  getGeneralProblem(void) const { return rejCode.general; }
  InvokeProblem_e   getInvokeProblem(void) const { return rejCode.invoke; }
  RResultProblem_e  getRResultProblem(void) const { return rejCode.rResult; }
  RErrorProblem_e   getRErrorProblem(void) const { return rejCode.rError; }
};


} //ros
} //eyeline

#endif /* __ROS_REJECT_PROBLEM_DEFS_HPP */

