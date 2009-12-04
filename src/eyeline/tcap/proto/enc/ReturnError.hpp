#ifndef __EYELINE_TCAP_PROTO_ENC_RETURNERROR_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_RETURNERROR_HPP__

# include <sys/types.h>

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/tcap/proto/enc/InvokeIdType.hpp"
# include "eyeline/tcap/proto/enc/OperationOpCode.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

enum { ReturnError_Max_Num_Of_Fields = 3 };

class ReturnError : public asn1::ber::EncoderOfSequence_T<ReturnError_Max_Num_Of_Fields> {
public:
  ReturnError(ros::InvokeId invoke_id, uint8_t op_code,
              asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);

  void setResult(void* result_value);

  void arrangeFields();

private:
  InvokeIdType* _invokeIdEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(InvokeIdType)];
  } _memForInvokeIdEncoder;

  Operation_OpCode* _errCodeEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(Operation_OpCode)];
  } _memForErrCodeEncoder;
};

}}}}

#endif
