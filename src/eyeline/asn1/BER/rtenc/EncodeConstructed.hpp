/* ************************************************************************* *
 * BER Encoder: core class for all constructed/structured types encoders
 *              (SET, SET OF, SEQUENCE, SEQUENCE OF).
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_CONSTRUCTED
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_CONSTRUCTED

#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the value of one of structured types (SET, SET OF,
 * SEQUENCE, SEQUENCE OF) according to X.690 clause 8.9, 8.10, 8.11, 8.12
 *
 * NOTE: It's a sucessor responsibility to perform ordering of elements
 * encodings implied by appropriate DER/CER restrictions.
 * ************************************************************************* */
class EncoderOfConstructedAC : public TypeValueEncoderAC {
public:
  typedef eyeline::util::LWArrayExtension_T<TLVLayoutEncoder, uint16_t> ElementsArray;

private:
  ElementsArray * _elmArray; //NOTE: actually it's a reference to storage,
                             //that is a successor member.

  //Determines required length determinant form and calculates total length
  //of fields encoding if possible.
  TSLength calculateElements(bool calc_indef = false) /*throw(std::exception)*/;

  //allocates (if necessary) and initializes encoder for field with given index
  TLVLayoutEncoder & reserveElement(uint16_t fld_idx) /*throw(std::exception)*/
  { //allocate (if necessary) and initialize field
    _elmArray->reserve(fld_idx + 1);
    return _elmArray->at(fld_idx);
  }
  //Returns address of FieldEncoder if associated field is initialized
  const TLVLayoutEncoder * hasElement(uint16_t fld_idx) const /*throw()*/
  {
    return ((fld_idx < _elmArray->size()) && (*_elmArray)[fld_idx].getVALEncoder())
            ? &(_elmArray->at(fld_idx)) : 0;
  }
  //Searches for first initialized FieldEncoder with index grater or
  //equal to specified one. Upon succesfull return 'fld_idx' is set
  //to returned field index, otherwise - maximum number of fields available.
  bool lookupNextElement(uint16_t & fld_idx) const  /*throw()*/
  {
    while ((fld_idx < _elmArray->size()) && !(*_elmArray)[fld_idx].getVALEncoder())
      ++fld_idx;
    return (fld_idx < _elmArray->size());
  }
  //Returns address of first initialized FieldEncoder with index
  //grater or equal to specified one. Upon succesfull return fld_idx
  //is set to returned field index, otherwise - maximum number of fields available.
  TLVLayoutEncoder * getNextElement(uint16_t & fld_idx) /*throw()*/
  {
    return lookupNextElement(fld_idx) ? &(*_elmArray)[fld_idx] : 0;
  }
  //
  const TLVLayoutEncoder * getNextElement(uint16_t & fld_idx) const /* throw()*/
  {
    return lookupNextElement(fld_idx) ? &(*_elmArray)[fld_idx] : 0;
  }

protected:
  //Initialization method for successors
  void initElement(uint16_t fld_idx, const ASTagging * fld_tags, ValueEncoderIface & val_enc)
  {
    reserveElement(fld_idx).init(val_enc, fld_tags, getVALRule());
  }
  void initElement(const ASTagging * fld_tags, ValueEncoderIface & val_enc)
  {
    reserveElement(_elmArray->size()).init(val_enc, fld_tags, getVALRule());
  }
  void clearElement(uint16_t fld_idx)
  {
    if (fld_idx < _elmArray->size())
      _elmArray->at(fld_idx).clear();
  }
  void clearElements(uint16_t start_idx = 0)
  {
    for (; start_idx < _elmArray->size(); ++start_idx)
      _elmArray->at(start_idx).clear();
  }

  ElementsArray * getElementsStorage(void) const { return _elmArray; }

  // -- -------------------------------------------------- --
  // -- ValueEncoderIface interface methods implementation
  // -- -------------------------------------------------- --
  //
  virtual void calculateVAL(TLVProperty & val_prop, TSGroupBER::Rule_e use_rule,
                            bool calc_indef = false) /*throw(std::exception)*/;
  //
  virtual ENCResult encodeVAL(uint8_t * use_enc,
                              TSLength max_len) const /*throw(std::exception)*/;

  //NOTE: the copying constructor of successsor MUST properly set _elmArray
  EncoderOfConstructedAC(const EncoderOfConstructedAC & use_obj)
    : TypeValueEncoderAC(use_obj), _elmArray(0)
  { }
  //
  void setElementsStorage(ElementsArray & use_store) { _elmArray = &use_store; }

public:
  //'Generic structured type encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  EncoderOfConstructedAC(ElementsArray & use_store, const ASTagging & eff_tags,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(eff_tags, use_rule)
    , _elmArray(&use_store)
  { }
  //'Generic tagged structured type encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  EncoderOfConstructedAC(ElementsArray & use_store, const ASTag & use_tag,
                       ASTagging::Environment_e tag_env,
                       const ASTagging & base_tags,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeValueEncoderAC(use_tag, tag_env, base_tags, use_rule)
    , _elmArray(&use_store)
  { }
  //
  virtual ~EncoderOfConstructedAC()
  { }
};


} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_CONSTRUCTED */

