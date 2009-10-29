#ifndef __EYELINE_TCAP_PROTO_ABORTMESSAGE_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ABORTMESSAGE_HPP__

# include <sys/types.h>
# include <netinet/in.h>
# include <inttypes.h>
# include "logger/Logger.h"
# include "eyeline/utilx/Exception.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"

# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/ASNTags.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeSequence.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"

# include "eyeline/tcap/proto/TCAPMessage.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class AbortMessage : public asn1::ber::EncoderOfSequence {
public:
  AbortMessage(TransactionId transaction_id, asn1::ber::TLVEncoderAC* reason)
  : _transactionId(transaction_id),
    _reason(reason)
  {}

  asn1::ENCResult encode(provd::UDTDataBuffer& used_buffer,
                         asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER) {
    asn1::ASTagging selectionTags(2, asn1::ASTag(asn1::ASTag::tagApplication, TCAPMessage::t_abort),
                                  asn1::_tagSEQOF);
    selectionTags.setEnvironment(ASTagging::tagsIMPLICIT);

    asn1::ber::EncoderOfSequence abort(selectionTags, use_rule);
    // prepare DestTransactionID
    asn1::ASTagging dtidTags(2, asn1::ASTag(asn1::ASTag::tagApplication, TCAPMessage::DestTrnIdTag),
                             asn1::_tagOCTSTR);
    dtidTags.setEnvironment(asn1::ASTagging::tagsIMPLICIT);
    uint32_t remoteTrnId = htonl(_transactionId.remoteId());
    asn1::ber::EncoderOfOCTSTR dtid(static_cast<uint16_t>(sizeof(remoteTrnId)),
                                    reinterpret_cast<uint8_t*>(&remoteTrnId),
                                    dtidTags);
    abort.addField(&dtid);

    // process OPTIONAL reason field
    if ( _reason )
      abort.addField(_reason);

    asn1::ber::EncoderOfChoice messageType;
    messageType.setSelection(&abort);
    return messageType.encodeTLV(used_buffer.getCurPtr(), used_buffer.getMaxSize());
  }

private:
  TransactionId _transactionId;
  asn1::ber::TLVEncoderAC* _reason;
};

}}}}

#endif
