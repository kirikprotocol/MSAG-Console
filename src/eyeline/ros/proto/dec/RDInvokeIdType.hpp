/* ************************************************************************* *
 * ROS Invokation Id type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_INVOKEIDTYPE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_INVOKEIDTYPE_HPP

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

/* InvokeIdType is defined in IMPLICIT tagging environment as follow:
   InvokeIdType ::= INTEGER(-128..127)
*/
class RDInvokeIdType : public asn1::ber::DecoderOfINTEGER {
private:
  using asn1::ber::DecoderOfINTEGER::setValue;

public:
  explicit RDInvokeIdType(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfINTEGER(use_rule)
  { }
  RDInvokeIdType(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfINTEGER(use_rule)
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

