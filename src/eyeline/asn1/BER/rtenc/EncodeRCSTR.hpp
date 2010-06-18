/* ************************************************************************* *
 * BER Encoder: Generic Restricted Character String encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_RCSTR
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_RCSTR

#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Converts value of Restricted Character String type to its octet string
 * representation performing necessary character transformations (escaping, etc)
 * ************************************************************************* */
class RCSEncConverterIface {
protected:
  virtual ~RCSEncConverterIface()
  { }

public:
  //Returns length of packed (converted) encoding of string
  virtual TSLength getPackedLength(void) = 0;
  //Incrementally converts next portion of string value writing requested
  //number of bytes to provided buffer.
  //Returns number of bytes written to buffer.
  virtual TSLength pack2Octs(uint8_t * use_buf, TSLength req_bytes) = 0;
};


class EncoderOfRCSTR : public TypeValueEncoderAC {
private:
  RCSEncConverterIface *  _strConv;
  TSLength                _encValSz;

protected:
  TSLength numFragments(void) const
  {
    return (_encValSz/1000 + (_encValSz%1000 ? 1 : 0));
  }

  //Returns total length of fragmented encoding
  //Throws if value is too large and cann't be encoded
  TSLength calculateFragments(void) const /*throw(std::exception)*/;

  // -- -------------------------------------- --
  // -- ValueEncoderIface interface methods
  // -- -------------------------------------- --
  virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                            bool calc_indef = false) /*throw(std::exception)*/;
  //
  virtual ENCResult encodeVAL(uint8_t * use_enc,
                               TSLength max_len) const /*throw(std::exception)*/;

public:
  //NOTE: tagging must be a complete tagging of type!
  EncoderOfRCSTR(RCSEncConverterIface * use_conv, const ASTagging & eff_tags,
                 TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(eff_tags, use_rule), _strConv(use_conv)
    , _encValSz(0)
  { }
  EncoderOfRCSTR(RCSEncConverterIface * use_conv, 
                 const ASTag & use_tag, ASTagging::Environment_e tag_env,
                 const ASTagging & base_tags,
                 TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, base_tags, use_rule), _strConv(use_conv)
    , _encValSz(0)
  { }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_RCSTR */

