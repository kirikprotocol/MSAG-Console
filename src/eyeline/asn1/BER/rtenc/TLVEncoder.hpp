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
extern ENCResult encode_tag(const ASTag & use_tag, uint8_t * use_enc,
                            TSLength max_len);


//TLV length determinant encoder
class LDEncoder {
public:
  enum Form_e { frmDefinite = 0, frmIndefinite };

protected:
  uint8_t _boc[(sizeof(TSLength)<<3 + 6)/7];
  uint8_t _bocSZO;  //number of 'begin-of-content' octets:
                    // - 1 in case of indefinite form,
                    // - [1 .. N] in case of definite form
  //NOTE: number of 'end-of-content' octets are predefined:
                    // - 2 in case of indefinite form,
                    // - 0 in case of definite form
  TSLength  _vlen;  //length of addressed value
  bool      _defForm;

public:
  LDEncoder(TSLength use_vlen = 0, Form_e use_form = frmDefinite)
    : _bocSZO(0), _vlen(use_vlen), _defForm(use_form == frmDefinite)
  {
    composeBOC();
  }
  ~LDEncoder()
  { }

  //Composes 'begin-of-content' octets of length determinant
  //encoding. Returns its length.
  uint8_t composeBOC(void);

  bool  isDefinite(void) const { return _defForm; }
  //Returns number of 'begin-of-content' octets
  uint8_t getBOCsize(void) { return _bocSZO ? _bocSZO : composeBOC(); }
  //Returns total number of length detrminant encoding  octets
  uint8_t getLDsize(void) { return getBOCsize() + getEOCsize(); }
  //Returns total number of length determinant encoding octets
  uint8_t getTLVsize(void) { return getLDsize() + _vlen; }
  //Returns number of 'end-of-content' octets
  uint8_t getEOCsize(void) const { return _defForm ? 0 : 2; }
  //Returns length of addressed value
  TSLength getValueLen(void) const { return _vlen; }

  void setForm(Form_e use_form = frmDefinite)
  {
    _defForm = (use_form == frmDefinite);
    composeBOC();
  }
  void setValueLen(TSLength use_vlen)
  {
    _vlen = use_vlen;
    composeBOC();
  }

  //Encodes 'begin-of-content' octets of length determinant encoding
  ENCResult encodeBOC(uint8_t * use_enc, TSLength max_len) const;
  //Encodes 'end-of-content' octets of length determinant encoding
  ENCResult encodeEOC(uint8_t * use_enc, TSLength max_len) const;
};

class TLPairEncoder : public ASTag, public LDEncoder {
protected:
  uint8_t _tldSZO;  //number of octets in 'TL'-part encoding, also
                    //serves as a 'calculation-performed' flag;

public:
  TLPairEncoder(const ASTag & use_tag = _tagUNI0,
         TSLength use_vlen = 0, Form_e use_form = frmDefinite)
    : ASTag(use_tag), LDEncoder(use_vlen, use_form), _tldSZO(0)
  { }
  ~TLPairEncoder()
  { }

  ENCResult encodeTL(uint8_t * use_enc, TSLength max_len) const;

  //Returns number of 'TL'-part octets ('begin-of-content')
  uint8_t getTLsize(void);
};

class TLVLayoutEncoder : public LWArray_T<TLPairEncoder, uint8_t, _ASTaggingDFLT_SZ> {
protected:
  TSLength  _tldSZO;       //overall length of 'TL'-part encoding, also
                            //serves as a 'calculation-performed' flag

public:
  TLVLayoutEncoder(const ASTag & use_tag)
    : LWArray_T<TLPairEncoder, uint8_t, _ASTaggingDFLT_SZ>(1), _tldSZO(0)
  {
    at(0) = use_tag;
  }
  //
  TLVLayoutEncoder(const ASTagging & use_tags)
    : LWArray_T<TLPairEncoder, uint8_t, _ASTaggingDFLT_SZ>(use_tags.size()), _tldSZO(0)
  {
    for (uint8_t i = 0; i < size(); ++i)
      at(i) = use_tags[i];
  }
  //
  TLVLayoutEncoder(const TLVLayoutEncoder & use_tlv)
    : LWArray_T<TLPairEncoder, uint8_t, _ASTaggingDFLT_SZ>(use_tlv), _tldSZO(0)
  { }

  ~TLVLayoutEncoder()
  { }

  TLPairEncoder & outerTL(void) { return at(0); }
  TLPairEncoder & innerTL(void) { return at(size()-1); }

  //Calculates TLV layout (length of 'TL' pair for each tag) basing on
  //given length of 'V' part encoding and length determinants form setting.
  //Returns  number of bytes of resulted 'TLV' encoding.
  TSLength calculate(void);
  //
  bool isCalculated(void) const { return (_tldSZO != 0); }
  //Returns true if TLVLayout uses ONLY indefinite form of length determinants.
  bool isIndefinite(void) const
  {
    for (uint8_t i = 0; i < size(); ++i) {
      if (get()[i].isDefinite())
        return false;
    }
    return true;
  }

  //Sets length of encoding of addressed type value
  void setValueLen(TSLength value_len)
  {
    innerTL().setValueLen(value_len);
    if (!isIndefinite()) //layout should be recalculated
      _tldSZO = 0;
  }
  //Returns length of encoding of addressed type value
  TSLength getValueLen(void) const
  {
    return get()[size()-1].getValueLen();
  }


  //Returns number of 'TL'-part octets ('begin-of-content')
  //Perfoms calculation of TLVLayout id necessary
  TSLength getTLsize(void)
  {
    return !isCalculated() ? calculate() : _tldSZO;
  }
  //Returns number of 'end-of-content' octets (EOC-part)
  TSLength getEOCsize(void) const;

  //Returns length of full TLV encoding.
  //Perfoms calculation of TLVLayout id necessary
  TSLength getTLVsize(void)
  {
    return getTLsize() + getValueLen() + getEOCsize();
  }

  //Encodes 'TL'-part ('begin-of-content' octets)
  //Perfoms calculation of TLVLayout id necessary
  ENCResult encodeTL(uint8_t * use_enc, TSLength max_len);
  //Encodes 'end-of-content' octets of encoding
  //NOTE: TLVLayout must be calculated.
  ENCResult encodeEOC(uint8_t * use_enc, TSLength max_len) const;
};


/* ************************************************************************* *
 * Abstract class that encodes by BER/DER/CER (composes TLV encoding) the
 * ASN.1 type value according to appropriate clause of X.690.
 * ************************************************************************* */
class TLVEncoderAC : public TLVLayoutEncoder, public TSGroupBER {
protected:
  using TLVLayoutEncoder::calculate;

  Rule_e    _rule;

public:
  TLVEncoderAC(const ASTag & use_tag,
               TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TLVLayoutEncoder(use_tag), _rule(use_rule)
  { }
  TLVEncoderAC(const ASTagging & use_tags,
               TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : TLVLayoutEncoder(use_tags), _rule(use_rule)
  { }
  virtual ~TLVEncoderAC()
  { }

  //Calculates TLV layout and overall length of BER/DER/CER encoding of type value.
  //NOTE: Throws in case of value that cann't be encoded.
  TSLength  calculateTLV(bool do_indef = true) /*throw(std::exception)*/
  {
    setValueLen(calculateVAL(do_indef));
    return getTLVsize();
  }

  //Encodes by BER/DER/CER (composes TLV) the type value.
  //Perfoms calculation of TLVLayout if necessary
  //NOTE: Throws in case of value that cann't be encoded.
  ENCResult encodeTLV(uint8_t * use_enc, TSLength max_len) /*throw(std::exception)*/
  {
    ENCResult rval = encodeTL(use_enc, max_len);
    if (rval.status == ENCResult::encOk) {
      rval += encodeVAL(use_enc + rval.nbytes, max_len - rval.nbytes);
      if (rval.status == ENCResult::encOk)
        rval += encodeEOC(use_enc + rval.nbytes, max_len - rval.nbytes);
    }
    return rval;
  }

  // -- ************************************* --
  // -- TLVEncoderAC interface methods
  // -- ************************************* --
  //Sets required kind of BER group encoding.
  //Returns: encoding rule effective for this type.
  //NOTE: may cause recalculation of TLVLayout due to restrictions
  //      implied by the DER or CER
  virtual TSGroupBER::Rule_e setRule(TSGroupBER::Rule_e use_rule);

  //Calculates length of BER/DER/CER encoding of type value.
  //If 'do_indef' flag is set, then length of value encoding is computed
  //even if TLVLayout uses only indefinite form of length determinants.
  //NOTE: may change TLVLayout if type value encoding should be fragmented.
  //NOTE: Throws in case of value that cann't be encoded.
  virtual TSLength  calculateVAL(bool do_indef = false) /*throw(std::exception)*/ = 0;

  //Encodes by BER/DER/CER the type value ('V'-part of encoding)
  //NOTE: Throws in case of value that cann't be encoded.
  virtual ENCResult encodeVAL(uint8_t * use_enc, TSLength max_len) const /*throw(std::exception)*/ = 0;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_ENCODER */

