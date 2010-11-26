/* ************************************************************************* *
 * ROS Primitives ARGUMENT decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_ARGUMENT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_ARGUMENT_HPP

#include "eyeline/ros/ROSPrimitives.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"


namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* ROS Primitives ARGUMENT is defined in IMPLICIT tagging environment as follow:
 
   Argument ::= ABSTRACT-SYNTAX.&Type({Operations})
 */
class RDPduArgument : public asn1::ber::DecoderOfASType {
private:
  using asn1::ber::DecoderOfASType::setValue;

public:
  explicit RDPduArgument(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfASType(use_rule)
  { }
  RDPduArgument(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfASType(outer_tag, tag_env, use_rule)
  { }
  //
  ~RDPduArgument()
  { }

  void setValue(PDUArgument & use_val) /*throw(std::exception)*/
  {
    use_val._kind = PDUArgument::asvTSyntax;
    asn1::ber::DecoderOfASType::setValue(use_val._tsEnc);
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_ARGUMENT_HPP */

