/* ************************************************************************* *
 * TCAP Message Decoder: TransactionId type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_DEC_TRANSACTIONID_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_DEC_TRANSACTIONID_HPP

#include <netinet/in.h>
#include "eyeline/asn1/BER/rtdec/DecodeOCTSTR.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace dec {

class TransactionIdDecoder : public asn1::ber::DecoderOfOCTSTR_T<uint8_t> {
private:
  using asn1::ber::DecoderOfOCTSTR_T<uint8_t>::setValue;

  typedef smsc::core::buffers::LWArray_T<uint8_t, uint8_t, 4> TrIdOcts;

  uint32_t * _dVal;
  TrIdOcts  _octs;

public:
  TransactionIdDecoder(const asn1::ASTag & use_tag, asn1::ASTagging::Environment_e tag_env,
                        asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfOCTSTR_T<uint8_t>(use_tag, tag_env, use_rule)
    , _dVal(0)
  {
    asn1::ber::DecoderOfOCTSTR_T<uint8_t>::setValue(_octs, 4);
  }
  ~TransactionIdDecoder()
  { }

  void setValue(uint32_t & use_val)   { _dVal = &use_val; }

  //Overriden TypeDecoderAC::decode(): converts OCTSTR to uint32_t
  virtual asn1::DECResult decode(const uint8_t * use_enc, asn1::TSLength max_len)
    /*throw(std::exception)*/;
};

/* OrigTransactionID is defined in IMPLICIT tagging environment as follow:
   OrigTransactionID ::= [APPLICATION 8]  OCTET STRING(SIZE (1..4))
*/
class TDOrigTransactionId : public TransactionIdDecoder {
public:
  static const asn1::ASTag _typeTag; //[APPLICATION 8] IMPLICIT

  explicit TDOrigTransactionId(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : TransactionIdDecoder(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  { }
  explicit TDOrigTransactionId(uint32_t & use_val,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : TransactionIdDecoder(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  {
    setValue(use_val);
  }
  ~TDOrigTransactionId()
  { }
};

/* DestTransactionID is defined in IMPLICIT tagging environment as follow:
  DestTransactionID ::= [APPLICATION 9]  OCTET STRING(SIZE (1..4))
*/
class TDDestTransactionId : public TransactionIdDecoder {
public:
  static const asn1::ASTag _typeTag;  //[APPLICATION 9] IMPLICIT

  explicit TDDestTransactionId(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : TransactionIdDecoder(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  { }
  explicit TDDestTransactionId(uint32_t & use_val,
                      asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : TransactionIdDecoder(_typeTag, asn1::ASTagging::tagsIMPLICIT, use_rule)
  {
    setValue(use_val);
  }
  ~TDDestTransactionId()
  { }
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_TRANSACTIONID_HPP */

