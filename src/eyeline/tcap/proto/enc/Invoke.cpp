#include "Invoke.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

const asn1::ASTagging Invoke::_linkedId_fieldTags =
    asn1::ASTagging(asn1::ASTag::tagContextSpecific,
                    Invoke::LinkedId_FieldTag,
                    asn1::ASTagging::tagsIMPLICIT);

Invoke::Invoke(ros::InvokeId invoke_id, uint8_t op_code,
               asn1::TSGroupBER::Rule_e use_rule)
: asn1::ber::EncoderOfSequence_T<Invoke_Msg_Max_Num_Of_Fields>(use_rule),
  _invokeIdEncoder(NULL), _linkedIdEncoder(NULL), _operationCodeEncoder(NULL)
{
  _invokeIdEncoder = new (_memForInvokeIdEncoder.allocation) InvokeIdType(invoke_id, use_rule);
  _operationCodeEncoder = new (_memForOperationCodeEncoder.allocation) Operation_OpCode(op_code, use_rule);
}

void
Invoke::setLinkedId(ros::InvokeId linked_id)
{
  _linkedIdEncoder = new (_memForLinkedIdEncoder.allocation) InvokeIdType(linked_id, getRule());
}

void
Invoke::arrangeFields()
{
  if ( !_invokeIdEncoder )
    throw utilx::SerializationException("Invoke::arrangeFields::: missed mandatory field 'invokeId'");
  addField(*_invokeIdEncoder);

  if ( _linkedIdEncoder )
    addField(*_linkedIdEncoder, &_linkedId_fieldTags);

  if ( !_operationCodeEncoder )
      throw utilx::SerializationException("Invoke::arrangeFields::: missed mandatory field 'operationCode'");
  addField(*_operationCodeEncoder);
}

}}}}
