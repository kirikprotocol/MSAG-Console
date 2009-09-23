/* ************************************************************************** *
 * Remote-Operations-Generic-ROS-PDUs according to 
 * joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
  * ************************************************************************** */
#ifndef __ROS_COMPONENT_DEFS_HPP
#ident "@(#)$Id$"
#define __ROS_COMPONENT_DEFS_HPP

#include "eyeline/asn1/AbstractSyntax.hpp"
#include "eyeline/ros/ROSRejectProblem.hpp"

namespace eyeline {
namespace ros {

using eyeline::asn1::EncodedOID;
using eyeline::asn1::ASTag;
using eyeline::asn1::ASTypeAC;
using eyeline::asn1::ASTypeRfp;
using eyeline::asn1::BITBuffer;
using eyeline::asn1::OCTBuffer;
using eyeline::asn1::ENCResult;
using eyeline::asn1::DECResult;
using eyeline::asn1::TransferSyntax;

typedef uint8_t InvokeId;

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
  InvokeId            _invId;  //related Invocation id
  const Kind_e        _kind;   //
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

  InvokeId getInvokeId(void) const { return _invId; }
  void setInvokeId(InvokeId inv_id) { _invId = inv_id; }
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
  InvokeId  _linkedId;
  bool      _linked;

public:
  ROSInvoke(uint8_t op_code, const EncodedOID * app_ctx = 0)
      : ROSComponentPrimitive(ROSComponentPrimitive::rosInvoke, op_code, app_ctx)
      , _linked(false), _linkedId(0)
  { }

  void setLinked(InvokeId linked_id)
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
  InvokeId getLinked(void) const { return _linkedId; }

  uint16_t getTimeout() const;
  void setTimeout(uint16_t timeout);

  enum OpClass_e { Class_1, Class_2, Class_3, Class_4 };
  OpClass_e getOperationClass() const;
  void setOperationClass(OpClass_e opClass);

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult encode(OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult decode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to OCTBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
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

  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;
  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult encode(OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult decode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to OCTBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
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
  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult encode(OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult decode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to OCTBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
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
  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult encode(OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult decode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to OCTBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;
};


class ROSReject : public ROSComponentPrimitive {
protected:
  RejectProblem   _problem;

public:
  ROSReject()
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejGeneral, 0)
    , _problem(RejectProblem::rjg_unrecognizedPDU)
  { }
  ROSReject(RejectProblem::GeneralProblem_e rjg_code)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejGeneral, 0)
    , _problem(rjg_code)
  { }
  ROSReject(RejectProblem::InvokeProblem_e rji_code)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejInvoke, 0)
    , _problem(rji_code)
  { }
  ROSReject(RejectProblem::RResultProblem_e rjr_code)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejResult, 0)
    , _problem(rjr_code)
  { }
  ROSReject(RejectProblem::RErrorProblem_e rje_code)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosReject, (uint8_t)RejectProblem::rejError, 0)
    , _problem(rje_code)
  { }

  const RejectProblem & Problem(void) const { return _problem; }

  // ---------------------------------
  // -- ASTypeAC interface methods
  // ---------------------------------
  using ASTypeAC::encode;
  using ASTypeAC::decode;
  using ASTypeAC::deferredDecode;

  //REQ: if use_rule == valRule, presentation > valNone, otherwise presentation == valDecoded
  ENCResult encode(OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valDecoded,
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult decode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;

  //REQ: presentation == valNone
  //OUT: presentation (include all subcomponents) = valMixed | valDecoded
  //NOTE: in case of valMixed keeps references to OCTBuffer !!!
  //NOTE: in case of decMoreInput, stores decoding context 
  DECResult deferredDecode(const OCTBuffer & use_buf, EncodingRule use_rule = TransferSyntax::ruleDER)
    /*throw ASN1CodecError*/;
};

} //ros
} //eyeline

#endif /* __ROS_COMPONENT_DEFS_HPP */

