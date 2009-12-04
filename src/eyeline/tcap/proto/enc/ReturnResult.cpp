#include "ReturnResult.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

ReturnResult::ReturnResult(ros::InvokeId invoke_id,
                           asn1::TSGroupBER::Rule_e use_rule)
: asn1::ber::EncoderOfSequence_T<ReturnResult_Max_Num_Of_Fields>(use_rule),
  _resultEncoder(NULL)
{
  _invokeIdEncoder = new (_memForInvokeIdEncoder.allocation) InvokeIdType(invoke_id, use_rule);
}

void
ReturnResult::setResult(uint8_t op_code, void* result_value)
{
  _resultEncoder = new (_memForResultEncoder.allocation) Result(op_code, result_value, getRule());
}

void
ReturnResult::arrangeFields()
{
  if ( !_invokeIdEncoder )
    throw utilx::SerializationException("ReturnResult::arrangeFields::: missed mandatory field 'invokeId'");
  addField(*_invokeIdEncoder);

  if ( _resultEncoder )
    addField(*_resultEncoder);
}

}}}}
