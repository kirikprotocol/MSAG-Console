/* ************************************************************************* *
 * ROS Local Operation code (LOCAL) type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_OPERATION_LOCAL_CODE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_OPERATION_LOCAL_CODE_HPP

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

/* LocalOperationCode is defined in EXPLICIT tagging environment as follow:
   LocalOperationCode ::= INTEGER
*/
class RELocalOpCode : public asn1::ber::EncoderOfINTEGER {
private:
  using asn1::ber::EncoderOfINTEGER::setValue;

public:
  explicit RELocalOpCode(asn1::TransferSyntax::Rule_e use_rule
                          = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfINTEGER(use_rule)
  { }
  RELocalOpCode(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfINTEGER(outer_tag, tag_env, use_rule)
  { }
  //
  ~RELocalOpCode()
  { }

  void setValue(const ros::LocalOpCode & use_val)
  {
    asn1::ber::EncoderOfINTEGER::setValue(use_val);
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_OPERATION_LOCAL_CODE_HPP */

