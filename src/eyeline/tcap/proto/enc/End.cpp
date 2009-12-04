#include "End.hpp"
#include "TransactionId.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

asn1::ENCResult
End::encode(provd::UDTDataBuffer& used_buffer,
            asn1::TSGroupBER::Rule_e use_rule)
{
  const uint16_t maxNumOfFields = 3;
  asn1::ber::EncoderOfSequence endEncoder(maxNumOfFields, use_rule);

  DestTransactionId dtid(_transactionId.remoteId(), use_rule);

  endEncoder.addField(dtid);

  if ( _componentPortion )
    endEncoder.addField(*_componentPortion);

  asn1::ber::EncoderOfChoice messageTypeEncoder(use_rule);

  asn1::ASTagging selectionTags(asn1::ASTag::tagApplication,
                                End_SelectionTag,
                                asn1::ASTagging::tagsIMPLICIT);

  messageTypeEncoder.setSelection(endEncoder, &selectionTags);
  return messageTypeEncoder.encodeTLV(used_buffer.getCurPtr(), used_buffer.getMaxSize());
}

}}}}
