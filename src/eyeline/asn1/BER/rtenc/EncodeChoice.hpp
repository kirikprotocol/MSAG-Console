/* ************************************************************************* *
 * BER Encoder methods: CHOICE type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_CHOICE
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_CHOICE

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {


class EncoderOfChoice : public TypeEncoderAC {
protected:
  FieldEncoder  _alt; //selected alternative

  // -- ***************************************** --
  // -- ValueEncoderAC abstract methods
  // -- ***************************************** --

  //Determines properties of addressed value encoding (LD form, constructedness)
  //according to requested encoding rule of BER family. Additionally calculates
  //length of value encoding if one of following conditions is fulfilled:
  // 1) LD form == ldDefinite
  // 2) (LD form == ldIndefinite) && ('calc_indef' == true)
  //NOTE: 'calc_indef' must be set if this encoding is enclosed by
  //another that uses definite LD form.
  //NOTE: Throws in case of value that cann't be encoded.
  const EncodingProperty & calculateVAL(bool calc_indef = false) /*throw(std::exception)*/;

  //Encodes by requested encoding rule of BER family the type value ('V'-part of encoding)
  //NOTE: Throws in case of value that cann't be encoded.
  //NOTE: this method has defined result only after calculateVAL() called
  ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/
  {
    return _alt.encodeCalculated(use_enc, max_len);
  }

public:
  // constructor for untagged CHOICE
  EncoderOfChoice(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TypeEncoderAC(use_rule)
  { }
  // constructor for tagged CHOICE
  EncoderOfChoice(const ASTagging & use_tags,
                  TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TypeEncoderAC(use_tags, use_rule)
  { }

  void setSelection(TypeEncoderAC & type_enc, const ASTagging * fld_tags = NULL)
  {
    _alt.init(type_enc, fld_tags);
    _alt.setRule(getRule());
  }

  // -- **************************************** --
  // -- ValueEncoderAC virtual methods
  // -- **************************************** --
  //Sets required kind of BER group encoding.
  //Returns: true if value encoding should be (re)calculated
  bool setRule(TSGroupBER::Rule_e use_rule);

  //Returns tag identifying the content of value encoding (not value type itself).
  //Defined only for so called 'hole types' such as untagged ANY, CHOICE, OpenType
  const ASTag * getContentTag(void) const { return _alt.getTag(); }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_CHOICE */

