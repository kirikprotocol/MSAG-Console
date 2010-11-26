/* ************************************************************************* *
 * BER Encoder: core class for all sequenced types encoders
 *              (SET OF, SEQUENCE OF).
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SEQUENCED
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_ENCODER_SEQUENCED

#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeConstructed.hpp"

#include "util/Exception.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

/* ************************************************************************* *
 * Encodes by BER/DER/CER the value of one of sequenced types (SET OF or
 * SEQUENCE OF) according to X.690 clause 8.10, 8.12
 *
 * NOTE: It's a sucessor responsibility to perform ordering of elements
 * encodings implied by appropriate DER/CER restrictions.
 * ************************************************************************* */
template <
  class _TArg
  //must have tagged & untagged type constructors
  , class _EncoderOfTArg /*: public TypeValueEncoder_T<_TArg>*/
  , uint16_t _NumElemsTArg //estimated number of element values
>
class EncoderOfSequencedAC_T : public EncoderOfConstructedAC {
private:
  typedef EncoderProducer_T<_EncoderOfTArg> EncoderProducer;
  typedef eyeline::util::LWArray_T<TLVLayoutEncoder, uint16_t, _NumElemsTArg> ElementsStore;
  typedef eyeline::util::LWArray_T<EncoderProducer, uint16_t, _NumElemsTArg> ProducersStore;

  ElementsStore   _elmStore;
  ProducersStore  _prdStore;

  uint16_t          _maxElems;  //maximum possible number of elements
  ASTagging         _elmTags;   //optional tagging of SE[Q|T] OF element

protected:
  _EncoderOfTArg * allocElementEncoder(void)
  {
    _prdStore.reserve(_prdStore.size() + 1);
    return _elmTags.empty() ? &(_prdStore.at(_prdStore.size()).init(getTSRule()))
                              : &(_prdStore.at(_prdStore.size()).init(_elmTags[0],
                                          _elmTags.getEnvironment(), getTSRule()));
  }
  //
  void reserveElementEncoders(uint16_t num_elems) /*throw(std::exception)*/
  {
    if (_maxElems && (num_elems >= _maxElems))
      throw smsc::util::Exception("asn1::ber::EncoderOfSequencedAC_T<%u>::reserveElements() - too much elements %u", _maxElems, num_elems);

    _prdStore.reserve(num_elems);
    EncoderOfConstructedAC::reserveElements(num_elems);
  }

  //NOTE: the copying constructor of successsor MUST properly set _prdArray
  EncoderOfSequencedAC_T(const EncoderOfSequencedAC_T & use_obj)
    : EncoderOfConstructedAC(use_obj), _elmStore(use_obj._elmStore),
      _prdStore(use_obj._prdStore), _maxElems(use_obj._maxElems), _elmTags(use_obj._elmTags)
  {
    setElementsStorage(_elmStore);
  }
  //

  //'Generic sequenced type encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  explicit EncoderOfSequencedAC_T(const ASTagging & eff_tags,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfConstructedAC(_elmStore, eff_tags, use_rule)
    , _maxElems(0)
  { }
  //'Generic tagged sequenced type encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  EncoderOfSequencedAC_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         const ASTagging & base_tags,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfConstructedAC(_elmStore, use_tag, tag_env, base_tags, use_rule)
    , _maxElems(0)
  { }

public:
  //
  virtual ~EncoderOfSequencedAC_T()
  { }

  // ----------------------------------------------------------
  // Initialization methods
  // ----------------------------------------------------------
  void setElementTag(const ASTag & elm_tag, ASTagging::Environment_e tag_env) /*throw()*/
  {
    _elmTags.init(elm_tag, tag_env);
  }

  void setMaxElements(uint16_t max_elems) /*throw()*/
  {
    _maxElems = max_elems;
  }

  //Adds a value of an element is to encode
  void addElementValue(const _TArg & use_val) /*throw(std::exception)*/
  {
    if (_maxElems && (_prdStore.size() >= _maxElems))
      throw smsc::util::Exception("asn1::ber::EncoderOfSequencedAC_T<%u>::addElementValue() - too much elements", _maxElems);

    _EncoderOfTArg * valEnc = allocElementEncoder();
    valEnc->setValue(use_val);
    initElement(valEnc->getTagging(), *(valEnc->getVALEncoder()));
  }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_SEQUENCED */

