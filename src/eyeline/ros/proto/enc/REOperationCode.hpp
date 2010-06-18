/* ************************************************************************* *
 * ROS Local Operation code (LOCAL) type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_OPERATION_LOCAL_CODE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_OPERATION_LOCAL_CODE_HPP

#include "eyeline/ros/ROSPdu.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* LocalOperationCode is defined in EXPLICIT tagging environment as follow:
   LocalOperationCode ::= INTEGER
*/
class RELocalOpCode : public asn1::ber::EncoderOfINTEGER {
private:
  using asn1::ber::EncoderOfINTEGER::setValue;

public:
  explicit RELocalOpCode(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfINTEGER(TSGroupBER::getTSRule(use_rule))
  { }
  RELocalOpCode(const ros::LocalOpCode & op_code,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfINTEGER(TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::EncoderOfINTEGER::setValue(op_code);
  }
  ~RELocalOpCode()
  { }

  void setValue(const ros::LocalOpCode & use_val)
  {
    asn1::ber::EncoderOfINTEGER::setValue(use_val);
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_OPERATION_LOCAL_CODE_HPP */

