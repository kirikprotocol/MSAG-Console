/* ************************************************************************** *
 * Remote-Operations-Generic-ROS-PDUs according to 
 * joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
  * ************************************************************************** */
#ifndef __ROS_COMPONENT_DEFS_HPP
#ident "@(#)$Id$"
#define __ROS_COMPONENT_DEFS_HPP

#include "eyeline/asn1/AbstractSyntax.hpp"

namespace eyeline {
namespace ros {

using eyeline::asn1::EncodedOID;
using eyeline::asn1::ASTag;
using eyeline::asn1::ASTypeAC;
using eyeline::asn1::ASTypeRfp;
using eyeline::asn1::BITBuffer;

//Base class for component of ROS OPERATION identified by local tag (uint8_t)
class ROSComponentPrimitive : public ASTypeAC {
public: 
  enum Kind_e { //ContextSpecific tag of component
    //basic components, according to
    //joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
    rosInvoke = 0x01    //[CONTEXT SPECIFIC 1]
    , rosResult = 0x02  //[CONTEXT SPECIFIC 2]
    , rosError = 0x03   //[CONTEXT SPECIFIC 3]
    , rosReject = 0x04  //[CONTEXT SPECIFIC 4]
    //additional conponent, according to
    //itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)
    , rosResultNL = 0x07  //[CONTEXT SPECIFIC 7]
    , rosCancel
  };

protected:
  uint8_t             _invId;  //related Invocation id
  const Kind_e        _kind;
  const uint8_t       _opCode; //component opcode or RejectProblem::ProblemKind
                               //in case of rosReject
  const EncodedOID *  _appCtx; //operation AC if defined, optional
  ASTypeRfp           _param;  //component parameter, optional

public: 
  ROSComponentPrimitive(Kind_e ros_kind, uint8_t op_code, const EncodedOID * app_ctx = 0)
      : ASTypeAC(ASTag::tagContextSpecific, ros_kind)
      , _invId(0xFF), _kind(ros_kind), _opCode(op_code), _appCtx(app_ctx), _param(0)
  { }

  Kind_e  rosKind(void) const { return _kind; }
  uint8_t opCode(void) const { return _opCode; }
  const EncodedOID *  appContext(void) const { return _appCtx; }

  uint8_t getInvokeId(void) const { return _invId; }
  void setInvokeId(uint8_t inv_id) { _invId = inv_id; }
  void setParam(const ASTypeRfp & ref_param) { _param = ref_param; }

  //Merges component paramater if it's was splitted to several
  //component due to segmentation issue
  virtual bool Merge(ROSComponentPrimitive & use_segm)
  {
    return false;
  }
};

class ROSInvoke : public ROSComponentPrimitive {
private:
  uint8_t _linkedId;
  bool    _linked;

public:
  ROSInvoke(uint8_t op_code, const EncodedOID * app_ctx = 0)
      : ROSComponentPrimitive(ROSComponentPrimitive::rosInvoke, op_code, app_ctx)
      , _linked(false), _linkedId(0)
  { }

  void setLinked(uint8_t linked_id)
  {
      _linked = true; _linkedId = linked_id;
  }
  void resetLinked(void)
  {
      _linked = false; _linkedId = 0;
  }

  bool hasLinked(void) const { return _linked; } 
  //NOTE: it's user responsibility to check for linked Invoke
  //status before using this method !
  uint8_t getLinked(void) const { return _linkedId; }

  uint16_t getTimeout() const;
  void setTimeout(uint16_t timeout);

  enum OpClass_e { Class_1, Class_2, Class_3, Class_4 };
  OpClass_e getOperationClass() const;
  void setOperationClass(OpClass_e opClass);

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;
};

class ROSResult : public ROSComponentPrimitive {
public:
  ROSResult(uint8_t op_code, const EncodedOID * app_ctx = 0)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosResult, op_code, app_ctx)
  { }
  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;
};

class ROSResultNL : public ROSComponentPrimitive {
public:
  ROSResultNL(uint8_t op_code, const EncodedOID * app_ctx = 0)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosResultNL, op_code, app_ctx)
  { }
  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;
};

class ROSError : public ROSComponentPrimitive {
public:
  ROSError(uint8_t op_code, const EncodedOID * app_ctx = 0)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosError, op_code, app_ctx)
  { }
  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;
};


class RejectProblem {
public:
  enum ProblemKind {
    rejGeneral = 0, rejInvoke = 1, rejResult = 2, rejError = 3
  };
  enum GeneralProblem {
    rjg_unrecognizedPDU = 0, rjg_mistypedPDU = 1, rjg_badlyStructuredPDU = 2
  };
  enum InvokeProblem {    //invocation problems
      rji_duplicateInvocation = 0, rji_unrecognizedOperation = 1
    , rji_mistypedArgument = 2, rji_resourceLimitation = 3
    , rji_releaseInProgress = 4, rji_unrecognizedLinkedId = 5
    , rji_linkedResponseUnexpected = 6, rji_unexpectedLinkedOperation = 7
  };
  enum RResultProblem {   //returnResult(NL) problems
      rjr_unrecognizedInvocation = 0, rjr_resultResponseUnexpected = 1
    , rjr_mistypedResult = 2
  };
  enum RErrorProblem {    //returnError problems
      rje_unrecognizedInvocation = 0, rje_errorResponseUnexpected =1
    , rje_unrecognizedError = 2, rje_unexpectedError = 3
    , rje_mistypedParameter = 4
  };

protected:
  const ProblemKind rejKind;

  union {
    GeneralProblem  general;
    InvokeProblem   invoke;
    RResultProblem  rResult;
    RErrorProblem   rError;
  } rejCode;

public:
  RejectProblem(GeneralProblem rjg_code)
      : rejKind(rejGeneral)
  {
    rejCode.general = rjg_code;
  }
  RejectProblem(InvokeProblem rji_code)
      : rejKind(rejInvoke)
  {
    rejCode.invoke = rji_code;
  }
  RejectProblem(RResultProblem rjr_code)
      : rejKind(rejResult)
  {
    rejCode.rResult = rjr_code;
  }
  RejectProblem(RErrorProblem rje_code)
      : rejKind(rejError)
  {
    rejCode.rError = rje_code;
  }

  ProblemKind     rejectKind(void) const { return rejKind; }

  GeneralProblem  generalProblem(void) const { return rejCode.general; }
  InvokeProblem   invokeProblem(void) const { return rejCode.invoke; }
  RResultProblem  rresultProblem(void) const { return rejCode.rResult; }
  RErrorProblem   rerrorProblem(void) const { return rejCode.rError; }
};

class ROSReject : public ROSComponentPrimitive {
protected:
  RejectProblem   _problem;

public:
  ROSReject()
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejGeneral, 0)
    , _problem(RejectProblem::rjg_unrecognizedPDU)
  { }
  ROSReject(RejectProblem::GeneralProblem rjg_code)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejGeneral, 0)
    , _problem(rjg_code)
  { }
  ROSReject(RejectProblem::RejectProblem::InvokeProblem rji_code)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejInvoke, 0)
    , _problem(rji_code)
  { }
  ROSReject(RejectProblem::RResultProblem rjr_code)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejResult, 0)
    , _problem(rjr_code)
  { }
  ROSReject(RejectProblem::RErrorProblem rje_code)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejError, 0)
    , _problem(rje_code)
  { }

  const RejectProblem & Problem(void) const { return _problem; }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult Encode(BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult Decode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to BITBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult DeferredDecode(const BITBuffer & use_buf, EncodingRule use_rule = ruleDER)
    /*throw ASN1CodecError*/;
};

} //ros
} //eyeline

#endif /* __ROS_COMPONENT_DEFS_HPP */

