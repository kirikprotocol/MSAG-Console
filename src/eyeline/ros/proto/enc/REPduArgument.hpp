/* ************************************************************************* *
 * ROS Primitives ARGUMENT encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_ARGUMENT_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_ENC_ARGUMENT_HPP

#include "eyeline/ros/ROSPrimitives.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"


namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* ROS Primitives ARGUMENT is defined in IMPLICIT tagging environment as follow:
 
   Argument ::= ABSTRACT-SYNTAX.&Type({Operations})
 */
class REPduArgument : public asn1::ber::EncoderOfASType {
private:
  using asn1::ber::EncoderOfASType::setValue;

public:
  explicit REPduArgument(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfASType(use_rule)
  { }
  REPduArgument(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfASType(outer_tag, tag_env, use_rule)
  { }
  //
  ~REPduArgument()
  { }

  void setValue(const PDUArgument & use_val) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_ARGUMENT_HPP */

