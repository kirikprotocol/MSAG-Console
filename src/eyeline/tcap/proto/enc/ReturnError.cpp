#include "ReturnError.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

ReturnError::ReturnError(ros::InvokeId invoke_id, uint8_t op_code,
                         asn1::TSGroupBER::Rule_e use_rule)
: asn1::ber::EncoderOfSequence_T<ReturnError_Max_Num_Of_Fields>(use_rule)
{
  _invokeIdEncoder = new (_memForInvokeIdEncoder.allocation) InvokeIdType(invoke_id, use_rule);
  _errCodeEncoder = new (_memForErrCodeEncoder.allocation) Operation_OpCode(op_code, use_rule);
}

void
ReturnError::arrangeFields()
{
  addField(*_invokeIdEncoder);
  addField(*_errCodeEncoder);
}

}}}}
