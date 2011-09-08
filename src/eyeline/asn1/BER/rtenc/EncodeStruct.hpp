/* ************************************************************************* *
 * BER Encoder: base class for structured types (SET, SEQUENCE) encoders.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_STRUCT
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_STRUCT

#include "eyeline/asn1/BER/rtenc/EncodeConstructed.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the value of structured type (SET, SEQUENCE)
 * according to X.690 clause 8.9, 8.11.
 *
 * NOTE: It's a sucessor responsibility to perform ordering of elements
 * encodings implied by appropriate DER/CER restrictions.
 * ************************************************************************* */
class EncoderOfStructureAC : public EncoderOfConstructedAC {
protected:
  //'Generic structured type encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  EncoderOfStructureAC(EncoderOfConstructedAC::ElementsArray & use_store,
                       const ASTagging & eff_tags,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfConstructedAC(use_store, eff_tags, use_rule)
  { }
  //'Generic tagged structured type encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  EncoderOfStructureAC(EncoderOfConstructedAC::ElementsArray & use_store,
                       const ASTag & use_tag, ASTagging::Environment_e tag_env,
                       const ASTagging & base_tags,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfConstructedAC(use_store, use_tag, tag_env, base_tags, use_rule)
  { }

  //Assigns untagged field/element with specified index
  void setField(uint16_t fld_idx, TypeEncoderAC & type_enc) /*throw(std::exception)*/
  {
    if (!type_enc.getTagging())
      throw smsc::util::Exception("ber::EncoderOfStructure::setField(): undefined element/field tagging");
    if (!type_enc.getVALEncoder())
      throw smsc::util::Exception("ber::EncoderOfStructure::setField(): undefined element/field ValueEncoder");

    initElement(fld_idx, type_enc.getTagging(), *type_enc.getVALEncoder());
  }
  //Appends untagged field/element to fields collection
  void addField(TypeEncoderAC & type_enc) /*throw(std::exception)*/
  {
    setField(getElementsStorage()->size(), type_enc);
  }
  //
  void clearField(uint16_t fld_idx) { clearElement(fld_idx); }
  //Clears fields starting from specified index up to last
  void clearFields(uint16_t fld_idx = 0) { clearElements(fld_idx); }

public:
  //
  virtual ~EncoderOfStructureAC()
  { }
};


//Template for Encoder of structured type with known number of fields/elements,
//All fields/elements housekeeping structures are allocated as class members at once
template <
  uint16_t _NumFieldsTArg     /* overall number of fields/elements */
>
class EncoderOfStructure_T : public EncoderOfStructureAC {
private:
  typedef smsc::core::buffers::LWArray_T<TLVLayoutEncoder, uint16_t, _NumFieldsTArg> FieldsStore;
  FieldsStore     _fieldsStore;

protected:
  //'Generic structured type encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  EncoderOfStructure_T(const ASTagging & eff_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructureAC(_fieldsStore, eff_tags, use_rule)
    , _fieldsStore(_NumFieldsTArg)
  { }
  //'Generic tagged structured type encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  EncoderOfStructure_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                       const ASTagging & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructureAC(_fieldsStore, use_tag, tag_env, base_tags, use_rule)
    ,  _fieldsStore(_NumFieldsTArg)
  { }
public:
  //
  virtual ~EncoderOfStructure_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_STRUCT */

