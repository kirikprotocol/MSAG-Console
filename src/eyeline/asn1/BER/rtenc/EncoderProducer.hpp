/* ************************************************************************* *
 * BER Encoder: helper template class for creation of TypeEncoders in 
 *              preallocated memory.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_PRODUCER_HPP

#include "util/OptionalObjT.hpp"

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/asn1/TransferSyntax.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  //must have tagged & untagged type constructors, copying constructor
  class _EncoderOfTArg /*: public TypeValueEncoder_T<_TArg>(TransferSyntax::Rule_e use_rule)*/ 
> 
class EncoderProducer_T : public smsc::util::OptionalObj_T<_EncoderOfTArg> {
public:
  EncoderProducer_T() : smsc::util::OptionalObj_T<_EncoderOfTArg>()
  { }
  ~EncoderProducer_T()
  { }

  //
  _EncoderOfTArg  & init(TransferSyntax::Rule_e use_rule)
  {
    this->clear();
    new (this->pObj())_EncoderOfTArg(use_rule);
    this->mHasObj = true;
    return *(this->pObj());
  }
  //
  _EncoderOfTArg  & init(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         TransferSyntax::Rule_e use_rule)
  {
    this->clear();
    new (this->pObj())_EncoderOfTArg(use_tag, tag_env, use_rule);
    this->mHasObj = true;
    return *(this->pObj());
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_PRODUCER_HPP */

