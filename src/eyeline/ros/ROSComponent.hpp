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
//using eyeline::asn1::ASTypeAC;
using eyeline::asn1::ASTypeRfp;

typedef uint8_t InvokeId;

//Base class for component of ROS OPERATION identified by local tag (uint8_t)
class ROSComponentPrimitive {
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
  const Kind_e        _kind;   //
  InvokeId            _invId;  //related Invocation id
  const uint8_t       _opCode; //component opcode(errcode) or 
                               //RejectProblem::ProblemKind_e in case of rosReject
  const EncodedOID *  _appCtx; //operation AppContext if defined, optional
  ASTypeRfp           _param;  //component parameter, optional

public: 
  ROSComponentPrimitive(Kind_e ros_kind,
                        uint8_t op_code, const EncodedOID * app_ctx = 0)
    : _kind(ros_kind), _invId(0xFF), _opCode(op_code), _appCtx(app_ctx), _param(0)
  { }

  Kind_e  rosKind(void) const { return _kind; }
  uint8_t opCode(void) const { return _opCode; }
  const EncodedOID *  appContext(void) const { return _appCtx; }

  InvokeId  getInvokeId(void) const { return _invId; }
  void      setInvokeId(InvokeId inv_id) { _invId = inv_id; }

  ASTypeRfp & getParam(void) const { return _param; }
  void        setParam(const ASTypeRfp & ref_param) { _param = ref_param; }

  //Merges component paramater if it's was splitted to several
  //component due to segmentation issue
  virtual bool Merge(ROSComponentPrimitive & use_segm)
  {
    return false;
  }
};

class ROSInvoke : public ROSComponentPrimitive {
public:
  //Operation classes according to X.219
  enum OpClass_e {
      opClass1_sync = 0 //synchronous, both success and failure are reported
    , opClass2_async    //asynchronous, both success and failure are reported
    , opClass3_async    //asynchronous, only failure is reported
    , opClass4_async    //asynchronous, only success is reported
    , opClass5_async    //asynchronous, neither success, nor failure is reported
  };

protected:
  OpClass_e _opClass;
  InvokeId  _linkedId;
  bool      _linked;

public:
  ROSInvoke(uint8_t op_code, const EncodedOID * app_ctx = 0)
      : ROSComponentPrimitive(ROSComponentPrimitive::rosInvoke, op_code, app_ctx)
      , _opClass(opClass5_async), _linked(false), _linkedId(0)
  { }

  void setLinked(InvokeId linked_id) { _linked = true; _linkedId = linked_id; }
  void resetLinked(void) { _linked = false; _linkedId = 0; }

  bool      hasLinked(void) const { return _linked; } 
  //NOTE: it's user responsibility to check for linked Invoke
  //status before using this method !
  InvokeId  getLinked(void) const { return _linkedId; }

  uint16_t  getTimeout(void) const;
  void      setTimeout(uint16_t timeout);

  OpClass_e getOperationClass(void) const { return _opClass; }
  void      setOperationClass(OpClass_e use_opClass) { _opClass = use_opClass; }
};

class ROSResult : public ROSComponentPrimitive {
public:
  ROSResult(uint8_t op_code, const EncodedOID * app_ctx = 0)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosResult, op_code, app_ctx)
  { }
};

class ROSResultNL : public ROSComponentPrimitive {
public:
  ROSResultNL(uint8_t op_code, const EncodedOID * app_ctx = 0)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosResultNL, op_code, app_ctx)
  { }
};

class ROSError : public ROSComponentPrimitive {
public:
  ROSError(uint8_t err_code, const EncodedOID * app_ctx = 0)
    : ROSComponentPrimitive(ROSComponentPrimitive::rosError, err_code, app_ctx)
  { }
};


class ROSReject : public ROSComponentPrimitive {
protected:
  RejectProblem   _problem;

  using ROSComponentPrimitive::setParam;

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

  const RejectProblem & getProblem(void) const { return _problem; }

};

} //ros
} //eyeline

#endif /* __ROS_COMPONENT_DEFS_HPP */

