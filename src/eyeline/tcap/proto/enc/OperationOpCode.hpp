#ifndef __EYELINE_TCAP_PROTO_ENC_OPERATIONOPCODE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_OPERATIONOPCODE_HPP__

# include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class Operation_OpCode : public asn1::ber::EncoderOfINTEGER {
public:
  explicit Operation_OpCode(uint8_t op_code,
                            asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER)
  : EncoderOfINTEGER(op_code, use_rule)
  {}
};

}}}}

#endif
