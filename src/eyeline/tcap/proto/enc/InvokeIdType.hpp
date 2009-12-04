#ifndef __EYELINE_TCAP_PROTO_ENC_INVOKEIDTYPE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_INVOKEIDTYPE_HPP__

# include "eyeline/ros/ROSComponent.hpp"
# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class InvokeIdType : public asn1::ber::EncoderOfINTEGER {
public:
  explicit InvokeIdType(ros::InvokeId invoke_id,
                        asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER)
  : EncoderOfINTEGER(invoke_id, use_rule)
  {}
};

}}}}

#endif
