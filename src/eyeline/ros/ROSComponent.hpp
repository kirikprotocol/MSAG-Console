/* ************************************************************************** *
 * Remote-Operations-Generic-ROS-PDUs according to 
 * joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
  * ************************************************************************** */
#ifndef __ROS_COMPONENT_DEFS_HPP
#ident "@(#)$Id$"
#define __ROS_COMPONENT_DEFS_HPP

#include "eyeline/asn1/ASNTypes.hpp"

namespace eyeline {
namespace ros {

using eyeline::asn1::EncodedOID;

//using eyeline::asn1::ASTag;
using eyeline::asn1::ASTypeAC;
//using eyeline::asn1::ASTypeTagging;
using eyeline::asn1::BITBuffer;

//Base class for component of ROS OPERATION
class ROSComponentAC : public ASTypeAC {
private:
    uint8_t     _invId;  //related Invocation id

public: 
    enum Kind_e { //ContextSpecific tag of component
        //basic components, according to
        //joint-iso-itu-t(2) remote-operations(4) generic-ROS-PDUs(6) version1(0)
        rosInvoke = 0x01
        , rosResult = 0x02
        , rosError = 0x03
        , rosReject = 0x04
        //additional conponent, according to
        //itu-t(0) recommendation(0) q(17) 773 modules(2) messages(1) version3(3)
        , rosResultNL = 0x07
    };

    const Kind_e        _kind;
    const uint8_t       _opCode; //component opcode or RejectProblem::ProblemKind
                                 //in case of rosReject
    const EncodedOID *  _appCtx; //operation AC if defined, optional

    ROSComponentAC(Kind_e ros_kind, uint8_t op_code, const EncodedOID * app_ctx = 0)
        : ASTypeAC(ASTag::tagContextSpecific, ros_kind)
        , _kind(ros_kind), _opCode(op_code), _appCtx(app_ctx), _invId(0xFF)
    { }

    void setInvokeId(uint8_t inv_id) { _invId = inv_id; }
    uint8_t invId(void) const { return _invId; }

    // ---------------------------------
    // ASTypeAC interface methods
    // ---------------------------------

    //REQ: presentation > valNone, if use_rule == valRule, otherwise presentation == valDecoded
    ENCResult Encode(BITBuffer & buf, EncodingRule use_rule) /*throw ASN1CodecError*/;
    //REQ: presentation == valEncoded | valMixed (setEncoding was called)
    //OUT: type presentation = valDecoded, components (if exist) presentation = valDecoded,
    //in case of decMoreInput, stores decoding context
    DECResult Decode(void) /*throw ASN1CodecError*/;
    //REQ: presentation == valEncoded (setEncoding was called)
    //OUT: type presentation = valMixed | valDecoded, 
    //     deferred components presentation = valEncoded
    //NOTE: if num_tags == 0, all components decoding is deferred 
    //in case of decMoreInput, stores decoding context 
    DECResult Demux(void) /*throw ASN1CodecError*/;
};

class ROSInvoke : public ROSComponentAC {
private:
    uint8_t _linkedId;
    bool    _linked;

public:
    ROSInvoke(uint8_t op_code, const EncodedOID * app_ctx = 0)
        : ROSComponentAC(ROSComponentAC::rosInvoke, op_code, app_ctx)
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
};

class ROSResult : public ROSComponentAC {
public:
    ROSResult(uint8_t op_code, const EncodedOID * app_ctx = 0)
        : ROSComponentAC(ROSComponentAC::rosResult, op_code, app_ctx)
    { }
};

class ROSResultNL : public ROSComponentAC {
public:
    ROSResultNL(uint8_t op_code, const EncodedOID * app_ctx = 0)
        : ROSComponentAC(ROSComponentAC::rosResultNL, op_code, app_ctx)
    { }
};

class ROSError : public ROSComponentAC {
public:
    ROSError(uint8_t op_code, const EncodedOID * app_ctx = 0)
        : ROSComponentAC(ROSComponentAC::rosError, op_code, app_ctx)
    { }
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
        , rji_releaseInProgress = 4, rji_unrecognizedLinkedId = 5,
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
    union {
        GeneralProblem  general;
        InvokeProblem   invoke;
        RResultProblem  rResult;
        RErrorProblem   rError;
    } rejCode;

public:
    const ProblemKind rejKind;

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

    GeneralProblem generalProblem(void) const { return rejCode.general; }
    InvokeProblem  invokeProblem(void) const { return rejCode.invoke; }
    RResultProblem rresultProblem(void) const { return rejCode.rResult; }
    RErrorProblem  rerrorProblem(void) const { return rejCode.rError; }
};

class ROSReject : public ROSComponentAC {
public:
    RejectProblem   problem;

    ROSReject(GeneralProblem rjg_code)
        : ROSComponentAC(ROSComponentAC::rosReject, (uint8_t)RejectProblem::rejGeneral, 0)
        , problem(rjg_code)
    { }
    ROSReject(InvokeProblem rji_code)
        : ROSComponentAC(ROSComponentAC::rosReject, (uint8_t)RejectProblem::rejInvoke, 0)
        , problem(rji_code)
    { }
    ROSReject(RResultProblem rjr_code)
        : ROSComponentAC(ROSComponentAC::rosReject, (uint8_t)RejectProblem::rejResult, 0)
        , problem(rjr_code)
    { }
    ROSReject(RErrorProblem rje_code)
        : ROSComponentAC(ROSComponentAC::rosReject, (uint8_t)RejectProblem::rejError, 0)
        , problem(rje_code)
    { }
};


} //ros
} //eyeline

#endif /* __ROS_COMPONENT_DEFS_HPP */

