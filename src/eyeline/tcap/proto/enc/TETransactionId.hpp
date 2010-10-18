/* ************************************************************************* *
 * TCAP Message Encoder: TransactionId type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_TRANSACTIONID_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROTO_ENC_TRANSACTIONID_HPP__

# include <netinet/in.h>
# include "eyeline/asn1/BER/rtenc/EncodeOCTSTR.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

class TransactionIdEncoder : public asn1::ber::EncoderOfOCTSTR {
private:
  using asn1::ber::EncoderOfOCTSTR::setValue;
  union {
    uint8_t * _buf;
    uint32_t  _num;
  } _trnIdValue;

public:
  explicit TransactionIdEncoder(uint32_t trn_id_value,
                                const asn1::ASTagging & type_tags,
                                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : EncoderOfOCTSTR(type_tags, use_rule)
  {
    setValue(trn_id_value);
  }
  ~TransactionIdEncoder()
  { }

  void setValue(uint32_t trn_id_value)
  {
    _trnIdValue._num = htonl(trn_id_value);
    asn1::ber::EncoderOfOCTSTR::setValue((asn1::TSLength)sizeof(_trnIdValue),
                                         _trnIdValue._buf);
  }
};

/* OrigTransactionID is defined in IMPLICIT tagging environment as follow:
   OrigTransactionID ::= [APPLICATION 8]  OCTET STRING(SIZE (1..4))
*/
class TEOrigTransactionId : public TransactionIdEncoder {
public:
  static const asn1::ASTagging _typeTags; //[APPLICATION 8] IMPLICIT

  explicit TEOrigTransactionId(uint32_t trn_id_value,
                             asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : TransactionIdEncoder(trn_id_value, _typeTags, use_rule)
  { }
  ~TEOrigTransactionId()
  { }
};

/* DestTransactionID is defined in IMPLICIT tagging environment as follow:
  DestTransactionID ::= [APPLICATION 9]  OCTET STRING(SIZE (1..4))
*/
class TEDestTransactionId : public TransactionIdEncoder {
public:
  static const asn1::ASTagging _typeTags;  //[APPLICATION 9] IMPLICIT

  explicit TEDestTransactionId(uint32_t trn_id_value,
                               asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : TransactionIdEncoder(trn_id_value, _typeTags, use_rule)
  { }
  ~TEDestTransactionId()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_TRANSACTIONID_HPP__ */

