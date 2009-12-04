#ifndef __EYELINE_TCAP_PROTO_ENC_RETURNRESULT_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_RETURNRESULT_HPP__

# include <sys/types.h>

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/tcap/proto/enc/InvokeIdType.hpp"
# include "eyeline/tcap/proto/enc/OperationOpCode.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

enum { ReturnResult_Max_Num_Of_Fields = 2, Result_Max_Num_Of_Fields = 2 };

class ReturnResult : public asn1::ber::EncoderOfSequence_T<ReturnResult_Max_Num_Of_Fields> {
public:
  ReturnResult(ros::InvokeId invoke_id,
               asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER);

  void setResult(uint8_t op_code, void* result_value);

  void arrangeFields();

private:
  InvokeIdType* _invokeIdEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(InvokeIdType)];
  } _memForInvokeIdEncoder;

  class Result : public asn1::ber::EncoderOfSequence_T<Result_Max_Num_Of_Fields> {
  public:
    Result(uint8_t op_code, void* result_value,
           asn1::TSGroupBER::Rule_e use_rule)
    : _operationCodeEncoder(op_code, use_rule)//,_resultEncoder(result_value, use_rule)
    {}
  private:
    Operation_OpCode _operationCodeEncoder;
    //Operation_ResType _resultEncoder;
  };

  Result* _resultEncoder;
  union {
    void* aligner;
    uint8_t allocation[sizeof(Result)];
  } _memForResultEncoder;
};

}}}}

#endif
