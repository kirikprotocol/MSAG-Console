#include "Begin.hpp"
#include "TransactionId.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

asn1::ENCResult
Begin::encode(provd::UDTDataBuffer& used_buffer,
              asn1::TSGroupBER::Rule_e use_rule)
{
  const uint16_t maxNumOfFields = 3;
  asn1::ber::EncoderOfSequence beginEncoder(maxNumOfFields, use_rule);

  OrigTransactionId otid(_transactionId.localId(), use_rule);

  beginEncoder.addField(otid);

  if ( _componentPortion )
    beginEncoder.addField(*_componentPortion);

  asn1::ber::EncoderOfChoice messageTypeEncoder(use_rule);

  asn1::ASTagging selectionTags(asn1::ASTag::tagApplication,
                                Begin_SelectionTag,
                                asn1::ASTagging::tagsIMPLICIT);

  messageTypeEncoder.setSelection(beginEncoder, &selectionTags);
  return messageTypeEncoder.encodeTLV(used_buffer.getCurPtr(), used_buffer.getMaxSize());
}

}}}}
