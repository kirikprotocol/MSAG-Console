/* ************************************************************************* *
 * ROS Local Operation code (LOCAL) type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_OPERATION_LOCAL_CODE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_OPERATION_LOCAL_CODE_HPP

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* LocalOperationCode is defined in EXPLICIT tagging environment as follow:
   LocalOperationCode ::= INTEGER
*/
class RDLocalOpCode : public asn1::ber::DecoderOfINTEGER {
private:
  using asn1::ber::DecoderOfINTEGER::setValue;

public:
  explicit RDLocalOpCode(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfINTEGER(TSGroupBER::getTSRule(use_rule))
  { }
  RDLocalOpCode(ros::LocalOpCode & op_code,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfINTEGER(TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::DecoderOfINTEGER::setValue(op_code);
  }
  //
  ~RDLocalOpCode()
  { }

  void setValue(ros::LocalOpCode & op_code)
  {
    asn1::ber::DecoderOfINTEGER::setValue(op_code);
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_OPERATION_LOCAL_CODE_HPP */

