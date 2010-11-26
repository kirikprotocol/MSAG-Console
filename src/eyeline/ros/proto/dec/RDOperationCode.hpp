/* ************************************************************************* *
 * ROS Local Operation code (LOCAL) type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_OPERATION_LOCAL_CODE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_OPERATION_LOCAL_CODE_HPP

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* LocalOperationCode is defined in EXPLICIT tagging environment as follow:
   LocalOperationCode ::= INTEGER
*/
class RDLocalOpCode : public asn1::ber::DecoderOfINTEGER {
private:
  using asn1::ber::DecoderOfINTEGER::setValue;

public:
  explicit RDLocalOpCode(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfINTEGER(use_rule)
  { }
  RDLocalOpCode(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfINTEGER(outer_tag, tag_env, use_rule)
  { }
  explicit RDLocalOpCode(ros::LocalOpCode & op_code,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfINTEGER(use_rule)
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

