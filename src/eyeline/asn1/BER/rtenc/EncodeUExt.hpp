/* ************************************************************************* *
 * BER Encoder: helper class for encoding of unknown extensions of structured 
 *              type, which are collected in std::list container.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_UNKNOWN_EXTENSIONS
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_UNKNOWN_EXTENSIONS

#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeASType.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeStruct.hpp"
#include "eyeline/asn1/UnknownExtensions.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  uint16_t _NumElemsTArg //number of Encoders to allocate initially on stack
> 
class EncoderOfUExtension_T {
private:
  typedef EncoderProducer_T<EncoderOfASType> EncoderProducer;
  typedef eyeline::util::LWArray_T<EncoderProducer, uint16_t, _NumElemsTArg> EncodersStore;

  EncodersStore   _encsStore;

  //
  EncoderOfASType * allocElementEncoder(asn1::TransferSyntax::Rule_e use_rule)
  {
    _encsStore.reserve(_encsStore.size() + 1);
    return &(_encsStore.atLast().init(use_rule));
  }

public:
  EncoderOfUExtension_T()
  { }
  //
  ~EncoderOfUExtension_T()
  { }

  //Returns field index is to use for field following last unknown extension.
  uint16_t setValue(const asn1::UnknownExtensions & use_val,
                EncoderOfStructureAC & struct_enc, uint16_t fld_idx)
    /*throw(std::exception)*/
  {
    for (asn1::UnknownExtensions::ValueList::const_iterator
          it = use_val._tsList.begin(); it != use_val._tsList.end(); ++it, ++fld_idx) {
      /**/
      EncoderOfASType * valEnc = allocElementEncoder(struct_enc.getTSRule());
      valEnc->setValue(*it);
      struct_enc.setField(fld_idx, *valEnc);
    }
    return fld_idx;
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_UNKNOWN_EXTENSIONS */

