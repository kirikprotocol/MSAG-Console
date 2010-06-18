/* ************************************************************************* *
 * ROS Invokation Id type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_INVOKEIDTYPE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_INVOKEIDTYPE_HPP

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* InvokeIdType is defined in IMPLICIT tagging environment as follow:
   InvokeIdType ::= INTEGER(-128..127)
*/
class REInvokeIdType : public asn1::ber::EncoderOfINTEGER {
private:
  using asn1::ber::EncoderOfINTEGER::setValue;

public:
  explicit REInvokeIdType(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfINTEGER(TSGroupBER::getTSRule(use_rule))
  { }
  REInvokeIdType(const ros::InvokeId & invoke_id,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfINTEGER(TSGroupBER::getTSRule(use_rule))
  {
    asn1::ber::EncoderOfINTEGER::setValue(invoke_id);
  }
  ~REInvokeIdType()
  { }

  void setValue(const ros::InvokeId & invoke_id)
  {
    asn1::ber::EncoderOfINTEGER::setValue(invoke_id);
  }
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_INVOKEIDTYPE_HPP */

