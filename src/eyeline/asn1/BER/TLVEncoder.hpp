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

using eyeline::asn1::TSLength;
using eyeline::asn1::ENCResult;


using eyeline::asn1::ASTag;
/* ************************************************************************* *
 * Estimates length of BER encoding of ASN.1 Tag.
 * Returns  number of bytes of resulted encoding.
 * ************************************************************************* */
extern uint16_t estimate_tag(const ASTag & use_tag);
/* ************************************************************************* *
 * Encodes by BER the ASN.1 Tag according to X.680 clause 8.1.2.
 * ************************************************************************* */
extern ENCResult encode_tag(const ASTag & use_tag, uint8_t * use_enc,
                            TSLength max_len);


using eyeline::asn1::ASTagging;

//BER encoding length determinant
class LDeterminant {
public:
  enum Form_e { frmDefinite = 0, frmIndefinite };

protected:
  uint8_t _boc[(sizeof(TSLength)<<3 + 6)/7];
  uint8_t _bocNum;  //number of 'begin-of-content' octets:
                    // - 1 in case of indefinite form,
                    // - [1 .. N] in case of definite form
  //NOTE: number of 'end-of-content' octets are predefined:
                    // - 2 in case of indefinite form,
                    // - 0 in case of definite form
  TSLength  _vlen;  //length of addressed value
  bool      _defForm;

  void composeBOC(void);

public:
  LDeterminant(TSLength use_vlen = 0, Form_e use_form = frmDefinite)
    : _bocNum(0), _vlen(use_vlen), _defForm(use_form)
  {
    composeBOC();
  }
  ~LDeterminant()
  { }

  void resetForm(Form_e use_form = frmDefinite)
  {
    _defForm = (use_form == frmDefinite);
    composeBOC();
  }
  //
  LDeterminant & operator= (TSLength use_vlen)
  {
    _vlen = use_vlen;
    composeBOC();
    return *this;
  }

  //Returns number of 'begin-of-content' octets
  uint8_t octetsBOC(void) const { return _bocNum; }
  //Returns number of 'end-of-content' octets
  uint8_t octetsEOC(void) const { return _defForm ? 0 : 2; }
  //Returns total number of length detrminant encoding  octets
  uint8_t octetsLD(void) const { return octetsBOC() + octetsEOC(); }
  //Returns total number of length determinant encoding octets
  uint8_t octetsTLV(void) const { return octetsLD() + _vlen; }

  //Encodes 'begin-of-content' octets of length determinant encoding
  ENCResult encodeBOC(uint8_t * use_enc, TSLength max_len) const;
  //Encodes 'end-of-content' octets of length determinant encoding
  ENCResult encodeEOC(uint8_t * use_enc, TSLength max_len) const;
};


class TLVLayout : public ASTagging {
protected:
  LDeterminant    _sDets[_max_STACK_TAGS];
  LDeterminant *  _dets;          //array of length determinants for each tag
  TSLength        _octsTLV;       //overall length of 'TLV' encoding, also
                                  //it's a 'calculation-performed' flag

public:
  TLVLayout(const ASTagging & use_tags)
    : ASTagging(use_tags), _dets(_sDets), _octsTLV(0)
  {
    if (_heapBuf)
      _dets = new LDeterminant[_numTags];
  }
  //
  TLVLayout(const TLVLayout & use_tags)
    : ASTagging(use_tags), _dets(_sDets), _octsTLV(use_tags._octsTLV)
  {
    if (_heapBuf)
      _dets = new LDeterminant[_numTags];
    //copy length determinants
    for (uint8_t i = 0; i < _numTags; ++i)
      _dets[i] = use_tags._dets[i];
  }

  ~TLVLayout()
  {
    if (_heapBuf)
      delete [] _dets;
  }

  //Sets 'length determinant' form for specified tag.
  //NOTE: the TLVLayout MUST be recalculated after this call !
  void setLDForm(uint8_t tag_idx, LDeterminant::Form_e use_form) //throw std::exception
  {
    if (tag_idx >= _numTags)
      throw smsc::util::Exception("tag index=%u is out of range=%u",
                                  (unsigned)tag_idx, (unsigned)_numTags);
    _dets[tag_idx].resetForm(use_form);
    _octsTLV = 0; //reset 'calculation-performed' flag
  }

  //Calculates TLV layout (length of 'TL' pair for each tag) basing on
  //given length of 'V' part encoding and length determinants form setting.
  //Returns  number of bytes of resulted 'TLV' encoding.
  TSLength calculateFor(TSLength value_len);

  //Returns length of full TLV encoding.
  //NOTE: Zero value means layout was not calculated!
  TSLength octetsTLV(void) const { return _octsTLV; };

  //Returns number of 'begin-of-content' octets ('TL'-part)
  TSLength octetsBOC(void) const
  {
    TSLength rval = 0;
    for (uint8_t i = 0; i < _numTags; ++i)
      rval += _dets[i].octetsBOC();
    return rval;
  }
  //Returns number of 'end-of-content' octets (EOC-part)
  TSLength octetsEOC(void) const
  {
    TSLength rval = 0;
    for (uint8_t i = 0; i < _numTags; ++i)
      rval += _dets[i].octetsEOC();
    return rval;
  }

  //Encodes 'begin-of-content' octets ('TL'-part of encoding)
  ENCResult encodeBOC(uint8_t * use_enc, TSLength max_len) const;
  //Encodes 'end-of-content' octets of encoding
  ENCResult encodeEOC(uint8_t * use_enc, TSLength max_len) const;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_ENCODER */

