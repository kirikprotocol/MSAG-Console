/* ************************************************************************* *
 * ROS ReturnError PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_RETURN_ERROR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_RETURN_ERROR_HPP

#include "eyeline/ros/ROSPrimitives.hpp"

#include "eyeline/ros/proto/dec/RDInvokeIdType.hpp"
#include "eyeline/ros/proto/dec/RDOperationCode.hpp"
#include "eyeline/ros/proto/dec/RDPduArgument.hpp"

#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
#include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS ReturnError PDU is defined in IMPLICIT tagging environment as follow:
ReturnError ::= SEQUENCE {
    invokeId	InvokeIdType,
    errcode	    INTEGER,
    parameter	ABSTRACT-SYNTAX.&Type({Errors}) OPTIONAL
} */
class RDRErrorPdu : public asn1::ber::DecoderOfSequence_T<3> {
protected:
  ROSErrorPdu *   _dVal;
  RDInvokeIdType  _invId;
  RDLocalOpCode   _errCode;
  //Optionals:
  asn1::ber::DecoderProducer_T<RDPduArgument> _argument;

  //Initializes ElementDecoder for this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

public:
  explicit RDRErrorPdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(use_rule)
    , _dVal(0), _invId(use_rule), _errCode(use_rule)
  {
    construct();
  }
  RDRErrorPdu(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<3>(outer_tag, tag_env, use_rule)
    , _dVal(0), _invId(use_rule), _errCode(use_rule)
  {
    construct();
  }
  //
  ~RDRErrorPdu()
  { }

  void setValue(ROSErrorPdu & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
  }
};


}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_RETURN_ERROR_HPP__ */

