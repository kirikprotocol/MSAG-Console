#include "Abort.hpp"
#include "TransactionId.hpp"

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

asn1::ENCResult
Abort::encode(provd::UDTDataBuffer& used_buffer,
              asn1::TSGroupBER::Rule_e use_rule)
{
   const uint16_t maxNumOfFields = 2;
   asn1::ber::EncoderOfSequence abortEncoder(maxNumOfFields, use_rule);

   DestTransactionId dtid(_transactionId.remoteId(), use_rule);
   abortEncoder.addField(dtid);

   // process OPTIONAL reason field
   if ( _reason )
     abortEncoder.addField(*_reason);

   asn1::ber::EncoderOfChoice messageTypeEncoder(use_rule);

   asn1::ASTagging selectionTags(asn1::ASTag::tagApplication,
                                 Abort_SelectionTag,
                                 asn1::ASTagging::tagsIMPLICIT);

   messageTypeEncoder.setSelection(abortEncoder, &selectionTags);
   return messageTypeEncoder.encodeTLV(used_buffer.getCurPtr(), used_buffer.getMaxSize());
 }

}}}}
