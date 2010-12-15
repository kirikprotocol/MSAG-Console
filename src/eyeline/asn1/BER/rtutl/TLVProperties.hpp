/* ************************************************************************* *
 * BER Runtime: 'TLV' elements/properites definitions.
 * ************************************************************************* */
#ifndef __ASN1_BER_TLV_PROPERTOES
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_TLV_PROPERTOES

#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

using eyeline::asn1::TSLength;

//Length determinant ('L'-part)
struct LDeterminant {
  enum Form_e { frmIndefinite = 0, frmDefinite = 1 };

  Form_e    _ldForm;  // 
  TSLength  _valLen;  //length of addressed value encoding,
                      //may be zero (undefined) in case of frmIndefinite

  explicit LDeterminant(Form_e use_form = frmIndefinite, TSLength use_vlen = 0)
    : _ldForm(use_form), _valLen(use_vlen)
  { }

  bool  isDefinite(void) const { return (_ldForm == frmDefinite); }
  bool  isIndefinite(void) const { return (_ldForm == frmIndefinite); }
};


//Properties of generic TLV encoding. ('LV'-part)
struct TLVProperty : public LDeterminant {
  bool      _isConstructed; //encoding constructedness

  //default constructor is for primitive encoding
  explicit TLVProperty(Form_e use_form = frmIndefinite, TSLength use_vlen = 0, bool use_construct = false)
    : LDeterminant(use_form, use_vlen), _isConstructed(use_construct)
  { }
  ~TLVProperty()
  { }

  void init(Form_e use_form, TSLength use_vlen, bool use_construct)
  {
    _isConstructed = use_construct; _ldForm = use_form; _valLen = use_vlen; 
  }
};


//Macro that determines maximum number of 'tag octets'
#define MAX_IDENTIFIER_OCTS(ident_type) (1 + ((sizeof(ident_type)<<3) + 6)/7)
//Macro that determines maximum number of 'length octets'
#define MAX_LDETERMINANT_OCTS(ldet_type) (1 + ((sizeof(ldet_type)<<3) + 7)/8)

//TLV Encoding structure (octets layout)
struct TLVStruct : public TLVProperty {
  uint8_t _szoTag; //number of 'tag octets'
  uint8_t _szoLOC; //number of 'length octets':
                   // - 1 in case of indefinite form of length determinant,
                   // - [1 .. N] in case of definite form
                   //NOTE: number of 'end-of-content' octets are predefined:
                   // - 2 in case of indefinite form of length determinant,
                   // - 0 in case of definite form

  TLVStruct() : TLVProperty(), _szoTag(0), _szoLOC(0)
  { }
  explicit TLVStruct(const TLVProperty & use_prop)
    : TLVProperty(use_prop), _szoTag(0), _szoLOC(0)
  { }

  //Returns number of 'begin-of-content' octets ('Tag & Length' octets)
  uint16_t getBOCsize(void) const { return (uint16_t)_szoTag + _szoLOC; }
  //Returns number of 'end-of-content' octets
  uint8_t getEOCsize(void) const { return (isDefinite() || !_szoTag) ? 0 : 2; }

  //Returns total length of TLV encoding if it's known (either definite
  //LD form or calculated _valLen)
  TSLength getTLVsize(void) const
  {
    return (_valLen || isDefinite()) ? (_valLen + getBOCsize() + getEOCsize()) : 0;
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_TLV_PROPERTOES */

