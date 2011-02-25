/* ************************************************************************* *
 * ROS Reject PDU decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_REJECT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_REJECT_HPP

#include "eyeline/ros/ROSPrimitives.hpp"
#include "eyeline/ros/proto/dec/RDInvokeIdType.hpp"
#include "eyeline/ros/proto/dec/RDProblemType.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS::Reject PDU is defined in IMPLICIT tagging environment as following:

Reject ::= SEQUENCE {
    invokeId	InvokeId,
    problem     ProblemType
} */
class RDRejectPdu : public asn1::ber::DecoderOfSequence_T<2> {
protected:
  ROSRejectPdu *  _dVal;
  RDInvokeIdType  _invId;
  RDProblemType   _problem;

  //Initializes ElementDecoder for this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfStructAC interface methods
  // ----------------------------------------
  //If necessary, allocates optional element and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

public:
  explicit RDRejectPdu(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(use_rule)
    , _dVal(0), _invId(use_rule), _problem(use_rule)
  {
    construct();
  }
  RDRejectPdu(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2>(outer_tag, tag_env, use_rule)
    , _dVal(0), _invId(use_rule), _problem(use_rule)
  {
    construct();
  }
  //
  ~RDRejectPdu()
  { }

  void setValue(ROSRejectPdu & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_REJECT_HPP__ */

