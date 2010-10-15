/* ************************************************************************* *
 * BER Encoder: base class for extensible structured types (SET, SEQUENCE) 
 *              encoders.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_STRUCT_EXT
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_STRUCT_EXT

#include "eyeline/asn1/UnknownExtensions.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeStruct.hpp"
#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

//Template for Encoder of extensible structured type with known number of fields/elements.
//All fields/elements housekeeping structures are allocated as class members at once

template <
  uint16_t _NumFieldsTArg     /* overall number of known fields/elements */
>
class EncoderOfExtensibleStructure_T : public EncoderOfStructureAC {
private:
  typedef eyeline::util::LWArray_T<TLVLayoutEncoder, uint16_t, _NumFieldsTArg + 1> FieldsStore;
  FieldsStore     _fieldsStore;

  typedef EncoderProducer_T<EncoderOfASType> ExtEncoderProducer;
  typedef eyeline::util::LWArray_T<ExtEncoderProducer, uint16_t, 1> ExtEncodersStore;

  ExtEncodersStore   _encsStore;

protected:
  //Returns field index is to use for field following the last unknown extension.
  uint16_t setExtensions(const asn1::UnknownExtensions & use_val, uint16_t fld_idx)
    /*throw(std::exception)*/
  {
    uint16_t numOfExt = (uint16_t)use_val._tsList.size();
    if (numOfExt) {
      if ((_NumFieldsTArg + numOfExt) < _NumFieldsTArg)
        throw smsc::util::Exception("asn1::ber::EncoderOfExtensibleStructure_T<%u>::setExtensions() - too much unknown extensions", _NumFieldsTArg);
  
      _fieldsStore.reserve(_NumFieldsTArg + numOfExt);
      _encsStore.reserve(numOfExt);
  
      for (asn1::UnknownExtensions::ValueList::const_iterator
            it = use_val._tsList.begin(); it != use_val._tsList.end(); ++it, ++fld_idx) {
        /**/
        EncoderOfASType * valEnc = &(_encsStore.at(_encsStore.size()).init(getTSRule()));
        valEnc->setValue(*it);
        this->setField(fld_idx, *valEnc);
      }
    }
    return fld_idx;
  }

  //'Generic structured type encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  EncoderOfExtensibleStructure_T(const ASTagging & eff_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructureAC(_fieldsStore, eff_tags, use_rule)
    , _fieldsStore(_NumFieldsTArg)
  { }
  //'Generic tagged structured type encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  EncoderOfExtensibleStructure_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                       const ASTagging & base_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfStructureAC(_fieldsStore, use_tag, tag_env, base_tags, use_rule)
    ,  _fieldsStore(_NumFieldsTArg)
  { }
public:
  //
  virtual ~EncoderOfExtensibleStructure_T()
  { }
};



} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_STRUCT_EXT */

