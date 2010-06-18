/* ************************************************************************* *
 * ROS Invokation Id type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_INVOKEIDTYPE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_INVOKEIDTYPE_HPP

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* InvokeIdType is defined in IMPLICIT tagging environment as follow:
   InvokeIdType ::= INTEGER(-128..127)
*/
class RDInvokeIdType : public asn1::ber::DecoderOfINTEGER {
private:
  using asn1::ber::DecoderOfINTEGER::setValue;

public:
  explicit RDInvokeIdType(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfINTEGER(TSGroupBER::getTSRule(use_rule))
  { }
  ~RDInvokeIdType()
  { }

  void setValue(ros::InvokeId & invoke_id)
  {
    asn1::ber::DecoderOfINTEGER::setValue(invoke_id);
  }
};

}}}}

#endif /* __EYELINE_ROS_PROTO_DEC_INVOKEIDTYPE_HPP */

