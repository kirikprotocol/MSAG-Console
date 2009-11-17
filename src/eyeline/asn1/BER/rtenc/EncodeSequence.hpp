/* ************************************************************************* *
 * BER Encoder methods: SEQUENCE/SEQUENCE OF types encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_ENCODESEQUENCE
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_ENCODESEQUENCE

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the SEQUENCE/SEQUENCE OF value according to X.690
 * clause 8.9, 8.10 (with appropriate DER/CER restrctions).
 * NOTE: if ASTagging is not set the standard [UNIVERSAL 16] tag goes to
 * resulting TLV encoding.
 * ************************************************************************* */
class EncoderOfSequenceAC : public TypeEncoderAC {
protected:
  uint16_t    _fldCnt;

  //Returns FieldEncoder for field with specified index
  //Throws on invalid field index
  virtual const FieldEncoder & getField(uint16_t fld_idx) const /* throw(std::exception)*/ = 0;

  //Calculates total length of fields encoding if possible
  TSLength calculateFields(bool calc_indef = false) const /*throw(std::exception)*/;

public:
  // constructor for untagged SEQUENCE/SEQUENCE OF with EXPLICIT tags environment
  EncoderOfSequenceAC(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TypeEncoderAC(ASTagging(_tagSEQOF), use_rule), _fldCnt(0)
  { }
  // constructor for tagged SEQUENCE/SEQUENCE OF
  EncoderOfSequenceAC(const ASTagging & use_tags,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TypeEncoderAC(use_tags, use_rule), _fldCnt(0)
  { }

  //Appends field to SEQUENCE
  virtual void addField(TypeEncoderAC & type_enc, const ASTagging * fld_tags = NULL) = 0;

  // -- **************************************** --
  // -- ValueEncoderAC virtual methods
  // -- **************************************** --
  //Sets required kind of BER group encoding.
  //Returns: true if value encoding should be (re)calculated
  bool setRule(TSGroupBER::Rule_e use_rule) const;

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
  const EncodingProperty & calculateVAL(bool calc_indef = false) const /*throw(std::exception)*/;

  //Encodes by requested encoding rule of BER family the type value ('V'-part of encoding)
  //NOTE: Throws in case of value that cann't be encoded.
  ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/;
};

//Template for Encoder of SEQUENCE with known number of fields.
//All fields are allocated on stack at once
template <uint16_t _NumFieldsTArg>
class EncoderOfSequence_T : public EncoderOfSequenceAC {
protected:
  typedef LWArray_T<FieldEncoder, uint16_t, _NumFieldsTArg> FieldList;

  FieldList   _fields;

  //Returns FieldEncoder for field with specified index
  //Throws on invalid field index
  const FieldEncoder & getField(uint16_t fld_idx) const /* throw(std::exception)*/
  {
    return _fields[fld_idx];
  }

public:
  // constructor for untagged SEQUENCE/SEQUENCE OF with EXPLICIT tags environment
  EncoderOfSequence_T(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : EncoderOfSequenceAC(use_rule), _fields(_NumFieldsTArg)
  { }

  // constructor for tagged SEQUENCE/SEQUENCE OF
  EncoderOfSequence_T(const ASTagging & use_tags,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : EncoderOfSequenceAC(use_tags, use_rule), _fields(_NumFieldsTArg)
  { }
  virtual ~EncoderOfSequence_T()
  { }

  //Appends field
  void addField(TypeEncoderAC & type_enc, const ASTagging * fld_tags)
  {
    _fields[_fldCnt].init(type_enc, fld_tags);
    _fields[_fldCnt++].setRule(getRule());
  }
};


//Generic Encoder of SEQUENCE.
//Memory for fields allocated at once either on heap or on stack (if <= 4)
class EncoderOfSequence : public EncoderOfSequenceAC {
protected:
  typedef LWArray_T<FieldEncoder, uint16_t, 4> FieldList;

  FieldList   _fields;

  //Returns FieldEncoder for field with specified index
  //Throws on invalid field index
  const FieldEncoder & getField(uint16_t fld_idx) const /* throw(std::exception)*/
  {
    return _fields[fld_idx];
  }

public:
  // constructor for untagged SEQUENCE/SEQUENCE OF with EXPLICIT tags environment
  EncoderOfSequence(uint16_t max_fields = 2, TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : EncoderOfSequenceAC(use_rule), _fields(max_fields)
  { }
  // constructor for tagged SEQUENCE/SEQUENCE OF
  EncoderOfSequence(const ASTagging & use_tags, uint16_t max_fields = 2,
                    TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : EncoderOfSequenceAC(use_tags, use_rule), _fields(max_fields)
  { }
  virtual ~EncoderOfSequence()
  { }

  //Appends field 
  void addField(TypeEncoderAC & type_enc, const ASTagging * fld_tags = NULL)
  {
    _fields[_fldCnt].init(type_enc, fld_tags);
    _fields[_fldCnt++].setRule(getRule());
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_ENCODESEQUENCE */

