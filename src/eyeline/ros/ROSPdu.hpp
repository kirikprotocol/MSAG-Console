/* ************************************************************************** *
 * Remote-Operations-Generic-ROS-PDUs according to 
 * joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
  * ************************************************************************** */
#ifndef __ROS_PDUS_DEFS_HPP
#ident "@(#)$Id$"
#define __ROS_PDUS_DEFS_HPP

#include "eyeline/asn1/AbstractValue.hpp"
#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/ros/ROSRejectProblem.hpp"

namespace eyeline {
namespace ros {

//Each ROS PDU consists of 3 parts:
// 1) header, identifying the Pdu and associated RemoteOperation, defined by X.880
// 2) Pdu dependent parameters, defined by X.880
// 3) Pdu argument (optional) - Remote Operation argument, provided by
//                              external layer that relies on ROS.

struct PduHeader {
  LocalOpCode   _opCode; //Operation local_code/error_code or 
                         //RejectProblem::ProblemKind_e in case of rosReject
  InvokeId      _invId;  //related Invocation id

  PduHeader() : _opCode(0), _invId(0)
  { }
  PduHeader(LocalOpCode op_code, InvokeId inv_id)
    : _opCode(op_code), _invId(inv_id)
  { }
};

struct PDUArgument {
  enum Representation_e {
      asvEmpty = 0x00
    , asvTSyntax = 0x01
    , asvDataValue = 0x02
    , asvDeferred = 0x03
  };

  Representation_e      _kind; //
  asn1::TransferSyntax  _tsEnc;
  asn1::ASTypeValueAC * _asType;

  PDUArgument() : _kind(asvEmpty), _asType(0)
  { }
  ~PDUArgument()
  { }

  bool empty(void) const { return _kind == asvEmpty; }
};


//Base abstract class for ROS PDU dealing with OPERATION identified by local tag (uint8_t)
class ROSPduPrimitiveAC : protected PduHeader {
public:
  enum Kind_e { //ContextSpecific tag of ROS PDU
    rosCancel = 0
    //basic pdus, according to
    //joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
    , rosInvoke = 0x01  //[CONTEXT SPECIFIC 1], has LinkedId as PDUParam
    , rosResult = 0x02  //[CONTEXT SPECIFIC 2]
    , rosError =  0x03  //[CONTEXT SPECIFIC 3]
    , rosReject = 0x04  //[CONTEXT SPECIFIC 4], has RejectProblem as PDUParam
    //additional pdu, according to
    //itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)
    , rosResultNL = 0x07  //[CONTEXT SPECIFIC 7]
  };
protected:
  Kind_e        _kind;

public: 
  ROSPduPrimitiveAC(Kind_e pdu_kind = rosCancel)
    : PduHeader(), _kind(pdu_kind)
  { }
  ROSPduPrimitiveAC(Kind_e pdu_kind, LocalOpCode op_code, InvokeId inv_id)
    : PduHeader(op_code, inv_id), _kind(pdu_kind)
  { }
  virtual ~ROSPduPrimitiveAC()
  { }

  static const char * nmKind(Kind_e ros_kind);

  const char * nmKind(void) const { return nmKind(_kind); }

  PduHeader & getHeader(void) { return (PduHeader &)*this; }
  const PduHeader & getHeader(void) const { return (const PduHeader &)*this; }
  
  Kind_e      getKind(void) const { return _kind; }
  bool        isInvoke(void) const { return _kind == rosInvoke; }

  LocalOpCode getOpCode(void) const { return _opCode; }
  InvokeId    getInvokeId(void) const { return _invId; }

  // ----------------------------------------
  // -- ROSPduPrimitiveAC interface methods
  // ----------------------------------------
  //Returns true if Pdu contains an optional argument type value.
  virtual bool hasArgument(void) const = 0;
};


class ROSPduWithArgument : public ROSPduPrimitiveAC {
protected:
  PDUArgument _arg;

public:
  ROSPduWithArgument(Kind_e pdu_kind = rosCancel)
    : ROSPduPrimitiveAC()
  { }
  ROSPduWithArgument(Kind_e pdu_kind, LocalOpCode op_code, InvokeId inv_id)
    : ROSPduPrimitiveAC(pdu_kind, op_code, inv_id)
  { }
  ~ROSPduWithArgument()
  { }

  PDUArgument & getArg(void) { return _arg; }
  const PDUArgument & getArg(void) const { return _arg; }

  // ----------------------------------------
  // -- ROSPduPrimitiveAC interface methods
  // ----------------------------------------
  //Returns true if Pdu contains an optional argument type value.
  virtual bool hasArgument(void) const
  {
    return _arg._kind != PDUArgument::asvEmpty;
  }
};

//NOTE: The X.880 replaces the old X.219 concept of OperationClasses by
//      introduction of the combination of two Operaton flags:
//       &synchronous          BOOLEAN DEFAULT FALSE,
//       &alwaysReturns        BOOLEAN DEFAULT TRUE,
// 
struct InvokeParm {
  enum FlagBit_e { bitSync = 0x01, bitReturn = 0x02 };
  enum OpFlags_e {
      asyncNoReturn = 0x00
    , asyncAlwaysReturn = 0x02  //default
    //NOTE: syncNoReturn = 0x01 is forbidden by X.880
    , syncReturn = 0x03
  };

  LinkedId    _linked;
  OpFlags_e   _opFlags;
  uint16_t    _timeOut; //NOTE: this parameter isn't defined by X.880,
                        //but is usefull for ROS Provider implementation.

  InvokeParm() : _opFlags(asyncAlwaysReturn), _timeOut(0)
  { }

  bool isAlwaysReturn(void) const { return (_opFlags & (uint8_t)bitReturn) != 0; }
  bool isSynchronous(void) const { return (_opFlags & (uint8_t)bitSync) != 0; }
};

class ROSInvokePdu : public ROSPduWithArgument {
protected:
  InvokeParm    _prm;

public:
  ROSInvokePdu()
    : ROSPduWithArgument(rosInvoke)
  { }
  ROSInvokePdu(LocalOpCode op_code, InvokeId inv_id)
    : ROSPduWithArgument(rosInvoke, op_code, inv_id)
  { }
  ~ROSInvokePdu()
  { }

  InvokeParm & getParam(void) { return _prm; }
  const InvokeParm & getParam(void) const { return _prm; }
};


class ROSResultPdu : public ROSPduWithArgument {
public:
  ROSResultPdu()
    : ROSPduWithArgument(rosResult)
  { }
  ROSResultPdu(LocalOpCode op_code, InvokeId inv_id)
    : ROSPduWithArgument(rosResult, op_code, inv_id)
  { }
  ~ROSResultPdu()
  { }
};

class ROSResultNLPdu : public ROSPduWithArgument {
public:
  ROSResultNLPdu()
    : ROSPduWithArgument(rosResultNL)
  { }
  ROSResultNLPdu(LocalOpCode op_code, InvokeId inv_id)
    : ROSPduWithArgument(rosResultNL, op_code, inv_id)
  { }
  ~ROSResultNLPdu()
  { }
};

class ROSErrorPdu : public ROSPduWithArgument {
public:
  ROSErrorPdu()
    : ROSPduWithArgument(rosResult)
  { }
  ROSErrorPdu(LocalOpCode err_code, InvokeId inv_id)
    : ROSPduWithArgument(rosResult, err_code, inv_id)
  { }
  ~ROSErrorPdu()
  { }
};


class ROSRejectPdu : public ROSPduPrimitiveAC {
protected:
  RejectProblem _problem;

public:
  ROSRejectPdu()
    : ROSPduPrimitiveAC(rosReject, (LocalOpCode)RejectProblem::rejGeneral, 0)
  { }
  ROSRejectPdu(RejectProblem::GeneralProblem_e rjg_code, InvokeId inv_id)
    : ROSPduPrimitiveAC(rosReject, (LocalOpCode)RejectProblem::rejGeneral, 0)
    , _problem(rjg_code)
  { }
  ROSRejectPdu(RejectProblem::InvokeProblem_e rji_code, InvokeId inv_id)
    : ROSPduPrimitiveAC(rosReject, (LocalOpCode)RejectProblem::rejInvoke, inv_id)
    , _problem(rji_code)
  { }
  ROSRejectPdu(RejectProblem::RResultProblem_e rjr_code, InvokeId inv_id)
    : ROSPduPrimitiveAC(rosReject, (LocalOpCode)RejectProblem::rejResult, inv_id)
    , _problem(rjr_code)
  { }
  ROSRejectPdu(RejectProblem::RErrorProblem_e rje_code, InvokeId inv_id)
    : ROSPduPrimitiveAC(rosReject, (LocalOpCode)RejectProblem::rejError, inv_id)
    , _problem(rje_code)
  { }
  //
  ~ROSRejectPdu()
  { }

  RejectProblem & getParam(void) { return _problem; }
  const RejectProblem & getParam(void) const { return _problem; }

  // ----------------------------------------
  // -- ROSPduPrimitiveAC interface methods
  // ----------------------------------------
  //Returns true if Pdu contains an optional argument type value.
  virtual bool hasArgument(void) const { return false; }
};

class ROSPdu {
private:
  union {
    void * _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf4_T<ROSInvokePdu, ROSResultPdu,
                                            ROSErrorPdu, ROSRejectPdu>::VALUE];
  } _memPdu;

protected:
  union {
    ROSPduPrimitiveAC * _any;
    ROSInvokePdu *    _invoke;
    ROSResultPdu *    _result;
    ROSErrorPdu *     _error;
    ROSRejectPdu *    _reject;
    ROSResultNLPdu *  _resultNL;
  } _value;

  void cleanUp(void)
  {
    if (_value._any) {
      _value._any->~ROSPduPrimitiveAC();
      _value._any = 0;
    }
  }
public:
  ROSPdu()
  {
    _value._any = 0;
  }
  ~ROSPdu()
  {
    cleanUp();
  }

  ROSPduPrimitiveAC::Kind_e getKind(void) const
  {
    return _value._any ? _value._any->getKind() : ROSPduPrimitiveAC::rosCancel;
  }

  const ROSPduPrimitiveAC * get(void) const { return _value._any; }

  ROSInvokePdu *    getInvoke(void) { return _value._invoke; }
  ROSResultPdu *    getResult(void) { return _value._result; }
  ROSErrorPdu *     getError(void)  { return _value._error; }
  ROSRejectPdu *    getReject(void) { return _value._reject; }
  ROSResultNLPdu *  getResultNL(void) { return _value._resultNL; }

  const ROSInvokePdu *    getInvoke(void) const { return _value._invoke; }
  const ROSResultPdu *    getResult(void) const { return _value._result; }
  const ROSErrorPdu *     getError(void) const  { return _value._error; }
  const ROSRejectPdu *    getReject(void) const { return _value._reject; }
  const ROSResultNLPdu *  getResultNL(void) const { return _value._resultNL; }


  ROSInvokePdu &    initInvoke(void)
  {
    cleanUp();
    _value._invoke = new (_memPdu._buf)ROSInvokePdu();
    return *_value._invoke;
  }
  //
  ROSResultPdu &    initResult(void)
  {
    cleanUp();
    _value._result = new (_memPdu._buf)ROSResultPdu();
    return *_value._result;
  }
  //
  ROSErrorPdu &     initError(void)
  {
    cleanUp();
    _value._error = new (_memPdu._buf)ROSErrorPdu();
    return *_value._error;
  }
  //
  ROSRejectPdu &    initReject(void)
  {
    cleanUp();
    _value._reject = new (_memPdu._buf)ROSRejectPdu();
    return *_value._reject;
  }
  //
  ROSResultNLPdu &  initResultNL(void)
  {
    cleanUp();
    _value._resultNL = new (_memPdu._buf)ROSResultNLPdu();
    return *_value._resultNL;
  }
};

} //ros
} //eyeline

#endif /* __ROS_PDUS_DEFS_HPP */

