/* ************************************************************************* *
 * DER Encoder: 'TL' parts encoding methods.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_ENCODER
#ident "@(#)$Id$"
#define __ASN1_BER_TLV_ENCODER

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::ASTag;
using eyeline::asn1::ASTagging;
using eyeline::asn1::TSLength;
using eyeline::asn1::ENCResult;

/* ************************************************************************* *
 * Estimates length of BER encoding of ASN.1 Tag.
 * Returns  number of bytes of resulted encoding.
 * ************************************************************************* */
extern uint8_t estimate_tag(const ASTag & use_tag);
/* ************************************************************************* *
 * Encodes by BER the ASN.1 Tag according to X.690 clause 8.1.2.
 * ************************************************************************* */
extern ENCResult encode_tag(const ASTag & use_tag, bool is_constructed,
                            uint8_t * use_enc, TSLength max_len);

//Length determinant
struct LDeterminant {
  enum Form_e { frmIndefinite = 0, frmDefinite = 1 };

  Form_e    _ldForm;  // 
  TSLength  _valLen;  //length of addressed value encoding,
                      //may be zero (undefined) in case of frmIndefinite

  LDeterminant(Form_e use_form = frmIndefinite, TSLength use_vlen = 0)
    : _ldForm(use_form), _valLen(use_vlen)
  { }

  bool  isDefinite(void) const { return (_ldForm == frmDefinite); }
};


//Properties of BER encoding of generic type value.
struct EncodingProperty : public LDeterminant {
  bool      _isConstructed; //encoding constructedness

  //default constructor is for primitive encoding
  EncodingProperty(Form_e use_form = frmIndefinite, TSLength use_vlen = 0, bool use_construct = false)
    : LDeterminant(use_form, use_vlen), _isConstructed(use_construct)
  { }

  void init(Form_e use_form, TSLength use_vlen, bool use_construct)
  {
    _isConstructed = use_construct; _ldForm = use_form; _valLen = use_vlen; 
  }
};


//TLV Encoding property
class TLVProperty : public EncodingProperty {
public:
  uint8_t _szoTag; //number of 'tag octets'
  uint8_t _szoLOC; //number of 'length octets':
                   // - 1 in case of indefinite form of length determinant,
                   // - [1 .. N] in case of definite form
                   //NOTE: number of 'end-of-content' octets are predefined:
                   // - 2 in case of indefinite form of length determinant,
                   // - 0 in case of definite form

  TLVProperty()
    : EncodingProperty(), _szoTag(0), _szoLOC(0)
  { }

  //Calculates number of octets in T and L parts
  void calculate(const ASTag & use_tag);

  //Returns number of 'begin-of-content' octets
  uint8_t getBOCsize(void) const { return _szoTag + _szoLOC; }
  //Returns number of 'end-of-content' octets
  uint8_t getEOCsize(void) const { return isDefinite() ? 0 : 2; }

  //Returns total length of TLV encoding if it's known (either definite
  //LD form or calculated _valLen) 
  TSLength getTLVsize(void) const
  {
    return (_valLen || isDefinite()) ? (_valLen + getBOCsize() + getEOCsize()) : 0;
  }
};


//Macro that determines maximum number of 'tag octets'
#define MAX_IDENTIFIER_OCTS(ident_type) (1 + (sizeof(ident_type)<<3 + 6)/7)
//Macro that determines maximum number of 'length octets'
#define MAX_LDETERMINANT_OCTS(ldet_type) (1 + (sizeof(ldet_type)<<3 + 7)/8)


/* ************************************************************************* *
 * Abstract class that determines encoding properties of the ASN.1 type value 
 * and composes the V-part of BER/DER/CER encoding according to appropriate
 * clause from X.690.
 * ************************************************************************* */
class ValueEncoderAC : public TSGroupBER {
protected:
  mutable Rule_e            _rule;
  mutable bool              _isCalculated;
  mutable EncodingProperty  _vProp;

public:
  ValueEncoderAC(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : _isCalculated(false), _rule(use_rule)
  { }
  virtual ~ValueEncoderAC()
  { }

  TSGroupBER::Rule_e   getRule(void) const { return _rule; }
  //NOTE: this method has defined result only after calculateVAL() called
  const EncodingProperty & getVALProperties(void) const { return _vProp; }

  // -- ************************************* --
  // -- ValueEncoderAC virtual methods
  // -- ************************************* --

  //Returns tag identifying the content of value encoding (not value type itself).
  //Defined only for so called 'hole types' such as untagged ANY, CHOICE, OpenType
  virtual const ASTag * getContentTag(void) const { return NULL; }

  //Sets required kind of BER group encoding.
  //Returns: true if value encoding should be (re)calculated
  virtual bool setRule(TSGroupBER::Rule_e use_rule) const
  {
    if (use_rule != _rule) {
      _rule = use_rule;
      _isCalculated = false;
    }
    return !_isCalculated;
  }

  // -- ************************************* --
  // -- ValueEncoderAC abstract methods
  // -- ************************************* --
  //Determines properties of addressed value encoding (LD form, constructedness)
  //according to requested encoding rule of BER family. Additionally calculates
  //length of value encoding if one of following conditions is fulfilled:
  // 1) LD form == ldDefinite
  // 2) (LD form == ldIndefinite) && ('calc_indef' == true)
  //NOTE: 'calc_indef' must be set if this encoding is enclosed by
  //another that uses definite LD form.
  //NOTE: Throws in case of value that cann't be encoded.
  virtual const EncodingProperty & calculateVAL(bool calc_indef = false) const /*throw(std::exception)*/ = 0;
  //Encodes by requested encoding rule of BER family the type value ('V'-part of encoding)
  //NOTE: Throws in case of value that cann't be encoded.
  virtual ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/ = 0;
};

//TL-part encoder
class TLEncoder : public TLVProperty {
protected:
  uint8_t _octTag[MAX_IDENTIFIER_OCTS(ASTag::ValueType)]; //4 bytes as max for uint16_t
  uint8_t _octLOC[MAX_LDETERMINANT_OCTS(TSLength)];       //5 bytes as max for uint32_t

public:
  TLEncoder() : TLVProperty()
  {
    _octTag[0] = _octLOC[0] = 0;
  }

  void reset(void)
  {
    init(LDeterminant::frmIndefinite, 0, false);
    _szoTag = _szoLOC = _octTag[0] = _octLOC[0] = 0;
  }

  void compose(const ASTag & use_tag);

  //Encodes 'begin-of-content' octets of TLV encoding
  ENCResult encodeBOC(uint8_t * use_enc, TSLength max_len) const;
  //Encodes 'end-of-content' octets of TLV encoding
  ENCResult encodeEOC(uint8_t * use_enc, TSLength max_len) const;
};


class TLVLayoutEncoder {
private:
  //data for encoding optimization if layout calculation was done prior to encoding
  mutable LWArray_T<TLEncoder, uint8_t, _ASTaggingDFLT_SZ> _tlws;
  mutable TSLength  _szoBOC; //overall length of 'begin-of-content' octets of TLV
                             //encoding, also serves as a 'calculation-performed' flag
protected:
  ASTagging         _effTags; //effective tags accroding to tagging environment
  ValueEncoderAC *  _valEnc;  //addressed value encoder, zero means layout isn't initialized


  void initTypeLayout(const ASTagging & use_tags, ValueEncoderAC & use_val_enc);
  void initFieldLayout(const TLVLayoutEncoder & type_enc, const ASTagging * fld_tags = NULL);

  //Calculates TLV layout (tag & length octets + EOC for each tag) basing on
  //given 'V' part encoding properties.
  //Returns  EncodingProperty for outermost tag
  const TLVProperty & calculateLayout(const EncodingProperty & val_prop) const;

  //Encodes 'TL'-part ('begin-of-content' octets)
  ENCResult encodeBOC(uint8_t * use_enc, TSLength max_len) const;
  //Encodes 'end-of-content' octets of encoding
  ENCResult encodeEOC(uint8_t * use_enc, TSLength max_len) const;

public:
  //Empty constructor: for later initialization by successor
  TLVLayoutEncoder()
    : _szoBOC(0), _valEnc(0)
  { }
  //'Untagged type layout encoder' constructor
  TLVLayoutEncoder(ValueEncoderAC & use_val_enc)
    : _szoBOC(0), _valEnc(&use_val_enc)
  { }
  //'Type layout encoder' constructor.
  //NOTE: tagging MUST contain UNIVERSAL tag of base type
  TLVLayoutEncoder(const ASTagging & use_tags, ValueEncoderAC & use_val_enc)
  {
    initTypeLayout(use_tags, use_val_enc);
  }
  //'Field layout encoder' constructor
  //NOTE: field tagging is optional
  TLVLayoutEncoder(const TLVLayoutEncoder & type_enc, const ASTagging * fld_tags = NULL)
  {
    initFieldLayout(type_enc, fld_tags);
  }

  //returns effective tagging of field/type value
  const ASTagging & getTagging(void) const { return _effTags; }

  //Returns the outermost tag of TLV layout
  //NOTE: may be NULL in case of incomplete value
  const ASTag * getTag(void) const
  {
    return _effTags.size() ? &_effTags.outerTag() : _valEnc->getContentTag();
  }
  //Returns addressed value encoder
  ValueEncoderAC * getValueEncoder(void) const { return _valEnc; }

  //Calculates TLV layout octets (length octets + EOC for each tag) basing
  //on encoding properties implied by addressed value.
  //Returns  EncodingProperty for outermost tag.
  // 
  //NOTE: if 'calc_indef' is set, then full TLV encoding length is calculated even
  //if indefinite LD form is used. This may be required if this TLV is enclosed by
  //another that uses definite LD form.
  const TLVProperty & calculateTLV(bool calc_indef = false) const /*throw(std::exception)*/
  {
    return calculateLayout(_valEnc->calculateVAL(calc_indef));
  }

  //Encodes by BER/DER/CER the type value (composes complete TLV encoding).
  //NOTE: Throws in case of value that cann't be encoded.
  ENCResult encodeTLV(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/;
};

/* ************************************************************************* *
 * Abstract class that encodes by BER/DER/CER (composes TLV encoding) the
 * ASN.1 type value according to appropriate clause of X.690.
 * ************************************************************************* */
class TypeEncoderAC : public TLVLayoutEncoder, public ValueEncoderAC {
public:
  //'Untagged type encoder' constructor
  TypeEncoderAC(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TLVLayoutEncoder((ValueEncoderAC&)*this), ValueEncoderAC(use_rule)
  { }
  //'Genric type encoder' constructor
  //NOTE: tagging MUST contain UNIVERSAL tag of base type
  TypeEncoderAC(const ASTagging & use_tags,
               TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TLVLayoutEncoder(use_tags, *this), ValueEncoderAC(use_rule)
  { }
  virtual ~TypeEncoderAC()
  { }

  // -- ************************************************* --
  // -- ValueEncoderAC abstract methods are to implement
  // -- ************************************************* --
  //virtual const EncodingProperty & calculateVAL(bool calc_indef = false) /*throw(std::exception)*/ = 0;
  //virtual ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/ = 0;
};


/* ************************************************************************* *
 * Encoder of field of constructed type.
 * ************************************************************************* */
class FieldEncoder : public TLVLayoutEncoder {
public:
  //Empty constructor for later initialization
  FieldEncoder() : TLVLayoutEncoder()
  { }
  //'Field encoder' constructor
  //NOTE: field tagging is optional
  FieldEncoder(TypeEncoderAC & type_enc, const ASTagging * fld_tags = NULL)
    : TLVLayoutEncoder(type_enc, fld_tags)
  { }
  ~FieldEncoder()
  { }
  //
  void init(TypeEncoderAC & type_enc, const ASTagging * fld_tags = NULL)
  {
    initFieldLayout(type_enc, fld_tags);
  }
  // -- --------------------------------------------------------------------
  // NOTE: following methods mat be called ONLY after proper initialization
  // -- --------------------------------------------------------------------
  bool setRule(TSGroupBER::Rule_e use_rule) const
  {
    return getValueEncoder()->setRule(use_rule);
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_ENCODER */

