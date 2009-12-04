#ifndef __EYELINE_TCAP_PROTO_ENC_TRANSACTIONID_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_TRANSACTIONID_HPP__

# include <sys/types.h>
# include <netinet/in.h>
# include <inttypes.h>

# include "eyeline/asn1/ASNTags.hpp"
# include "eyeline/asn1/TransferSyntax.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class TransactionIdEncoder : public asn1::ber::EncoderOfOCTSTR {
public:
  explicit TransactionIdEncoder(uint32_t trn_id_value,
                                asn1::ASTagging type_tags,
                                asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER)
  : EncoderOfOCTSTR(static_cast<uint16_t>(sizeof(_trnIdValue)),
                    reinterpret_cast<uint8_t*>(&_trnIdValue), type_tags, use_rule)
  {
    _trnIdValue = htonl(trn_id_value);
  }
private:
  uint32_t _trnIdValue;
};

class OrigTransactionId : public TransactionIdEncoder {
public:
  explicit OrigTransactionId(uint32_t trn_id_value,
                             asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER)
  : TransactionIdEncoder(trn_id_value, _typeTags, use_rule)
  {}

private:
  enum TypeTags_e {
    ORIG_TRN_ID_Tag = 8
  };

  static const asn1::ASTagging _typeTags;
};

class DestTransactionId : public TransactionIdEncoder {
public:
  explicit DestTransactionId(uint32_t trn_id_value,
                             asn1::TSGroupBER::Rule_e use_rule = asn1::TSGroupBER::ruleDER)
  : TransactionIdEncoder(trn_id_value, _typeTags, use_rule)
  {}

private:
  enum TypeTags_e {
    DEST_TRN_ID_Tag = 9
  };

  static const asn1::ASTagging _typeTags;
};

}}}}

#endif
