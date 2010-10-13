/* ************************************************************************* *
 * BER Decoder: helper template class for creation of TypeDecoders in 
 *              preallocated memory.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_PRODUCER_HPP
#ident "@(#)$Id$"
#define __ASN1_BER_DECODER_PRODUCER_HPP

#include "eyeline/util/OptionalObjT.hpp"

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  //must have tagged & untagged type constructors, copying constructor
  class _DecoderOfTArg /*: public TypeValueDecoder_T<_TArg>(TransferSyntax::Rule_e use_rule)*/ 
> 
class DecoderProducer_T : public eyeline::util::OptionalObj_T<_DecoderOfTArg> {
public:
  DecoderProducer_T() : eyeline::util::OptionalObj_T<_DecoderOfTArg>()
  { }
  ~DecoderProducer_T()
  { }

  //
  _DecoderOfTArg  & init(TransferSyntax::Rule_e use_rule)
  {
    this->clear();
    return *(this->_ptr = new (this->_mem._buf)_DecoderOfTArg(use_rule));
  }
  //
  _DecoderOfTArg  & init(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         TransferSyntax::Rule_e use_rule)
  {
    this->clear();
    return *(this->_ptr = new (this->_mem._buf)_DecoderOfTArg(use_tag, tag_env, use_rule));
  }

};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_DECODER_PRODUCER_HPP */

