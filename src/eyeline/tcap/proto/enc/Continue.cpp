#include "Continue.hpp"
#include "TransactionId.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

asn1::ENCResult
Continue::encode(provd::UDTDataBuffer& used_buffer,
                 asn1::TSGroupBER::Rule_e use_rule)
{
  const uint16_t maxNumOfFields = 4;
  asn1::ber::EncoderOfSequence continueEncoder(maxNumOfFields, use_rule);

  OrigTransactionId otid(_transactionId.localId(), use_rule);
  DestTransactionId dtid(_transactionId.remoteId(), use_rule);

  continueEncoder.addField(otid);
  continueEncoder.addField(dtid);

  if ( _componentPortion )
    continueEncoder.addField(*_componentPortion);

  asn1::ber::EncoderOfChoice messageTypeEncoder(use_rule);

  asn1::ASTagging selectionTags(asn1::ASTag::tagApplication,
                                Continue_SelectionTag,
                                asn1::ASTagging::tagsIMPLICIT);

  messageTypeEncoder.setSelection(continueEncoder, &selectionTags);
  return messageTypeEncoder.encodeTLV(used_buffer.getCurPtr(), used_buffer.getMaxSize());
}

}}}}
