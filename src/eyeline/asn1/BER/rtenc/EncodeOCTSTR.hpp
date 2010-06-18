/* ************************************************************************* *
 * BER Encoder: OCTET STRING type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_OCTSTR
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_OCTSTR

#include "eyeline/asn1/OCTSTR.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeRCSTR.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the OCTET STRING value according to X.690
 * clause 8.7 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 4] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfOCTSTR : public EncoderOfRCSTR {
protected:
  class OCTSTRConverter : public RCSEncConverterIface {
  protected:
    TSLength        _encValSz;
    const uint8_t * _encVal;
    TSLength        _numConverted;

  public:
    OCTSTRConverter(TSLength use_sz = 0, const uint8_t * use_octs = 0)
      : _encValSz(use_sz), _encVal(use_octs), _numConverted(0)
    { }

    void init(TSLength use_sz, const uint8_t * use_octs)
    {
      _encValSz = use_sz; _encVal = use_octs; _numConverted = 0;
    }

    //Returns length of resulted (converted) encoding of string
    TSLength getPackedLength(void) { return _encValSz; }

    //Incrementally converts next portion of string value writing requested
    //number of bytes to provided buffer.
    //Returns number of bytes written to buffer.
    TSLength pack2Octs(uint8_t * use_buf, TSLength req_bytes);
  };

  OCTSTRConverter   _valConv;

  // constructor for encoder of tagged type referencing OCTET STRING
  // NOTE: eff_tags must be a complete tagging of type! 
  EncoderOfOCTSTR(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, eff_tags, use_rule)
    , _valConv(0, 0)
  { }

public:
  //Constructors for OCTET STRING type
  EncoderOfOCTSTR(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, asn1::_tagsOCTSTR, use_rule)
    , _valConv(0, 0)
  { }
  EncoderOfOCTSTR(TSLength use_sz, const uint8_t * use_octs,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, asn1::_tagsOCTSTR, use_rule)
    , _valConv(use_sz, use_octs)
  { }

  //Constructors for tagged OCTET STRING type
  EncoderOfOCTSTR(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, use_tag, tag_env, asn1::_tagsOCTSTR, use_rule)
    , _valConv(0, 0)
  { }
  EncoderOfOCTSTR(TSLength use_sz, const uint8_t * use_octs,
                  const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfRCSTR(&_valConv, use_tag, tag_env, asn1::_tagsOCTSTR, use_rule)
    , _valConv(use_sz, use_octs)
  { }
  //
  ~EncoderOfOCTSTR()
  { }


  void setValue(const asn1::OCTArrayTiny & use_val) /*throw(std::exception)*/
  {
    _valConv.init((TSLength)use_val.size(), use_val.get());
  }
  //
  void setValue(const asn1::OCTArray64K & use_val) /*throw(std::exception)*/
  {
    _valConv.init((TSLength)use_val.size(), use_val.get());
  }
  //
  void setValue(TSLength use_sz, const uint8_t * use_octs) /*throw(std::exception)*/
  {
    _valConv.init(use_sz, use_octs);
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_OCTSTR */

