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

/* InvokeIdType is defined in IMPLICIT tagging environment as follow:
   InvokeIdType ::= INTEGER(-128..127)
*/
class REInvokeIdType : public asn1::ber::EncoderOfINTEGER {
private:
  using asn1::ber::EncoderOfINTEGER::setValue;

public:
  explicit REInvokeIdType(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfINTEGER(use_rule)
  { }
  REInvokeIdType(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfINTEGER(outer_tag, tag_env, use_rule)
  { }
  //
  ~REInvokeIdType()
  { }

  void setValue(const ros::InvokeId & invoke_id)
  {
    asn1::ber::EncoderOfINTEGER::setValue(invoke_id);
  }
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_INVOKEIDTYPE_HPP */

