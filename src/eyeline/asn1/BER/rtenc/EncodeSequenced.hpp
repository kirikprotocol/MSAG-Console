/* ************************************************************************* *
 * BER Encoder: core class for all sequenced types encoders
 *              (SET OF, SEQUENCE OF).
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_SEQUENCED
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_SEQUENCED

#include "eyeline/asn1/BER/rtenc/EncodeConstructed.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

template <
  //must have tagged & untagged type constructors, copying constructor
  class _EncoderOfTArg /*: public TypeValueEncoder_T<_TArg>*/ 
> 
class EncoderProducer_T {
private:
  union {
    void *    _aligner;
    uint8_t   _buf[sizeof(_EncoderOfTArg)];
  } _mem;

protected:
  _EncoderOfTArg  * _ptr;

  void cleanUp(void)
  {
    if (_ptr)
      _ptr->~_EncoderOfTArg();
  }
public:
  EncoderProducer_T() : _ptr(0)
  {
    _mem._aligner = 0;
  }
  explicit EncoderProducer_T(const EncoderProducer_T & use_obj) : _ptr(0)
  {
    _mem._aligner = 0;
    if (use_obj.get())
      _ptr = new (_mem._buf)_EncoderOfTArg(*use_obj.get());
  }
  //
  ~EncoderProducer_T()
  {
    cleanUp();
  }

  //
  _EncoderOfTArg  * init(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  {
    cleanUp();
    return _ptr = new (_mem._buf)_EncoderOfTArg(use_rule);
  }
  //
  _EncoderOfTArg  * init(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
  {
    cleanUp();
    return _ptr = new (_mem._buf)_EncoderOfTArg(use_tag, tag_env, use_rule);
  }

  //
  _EncoderOfTArg  * get(void) const { return _ptr; }
};

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
>
class EncoderOfSequencedAC_T : public EncoderOfConstructedAC {
public:
  typedef EncoderProducer_T<_EncoderOfTArg> EncoderProducer;
  typedef eyeline::util::LWArrayExtension_T<EncoderProducer, uint16_t> ProducersArray;

  typedef EncoderOfConstructedAC::ElementsArray ElementsArray; 

private:
  ProducersArray * _prdArray; //NOTE: actually it's a reference to storage,
                              //that is a successor member.
  ASTagging       _elmTags;   //optional tagging of SE[Q|T] OF element

protected:
  _EncoderOfTArg * allocElementEncoder(void)
  {
    _prdArray->reserve(_prdArray->size() + 1);
    return _elmTags.empty() ? _prdArray->at(_prdArray->size()).init(getVALRule())
                              : _prdArray->at(_prdArray->size()).init(_elmTags[0],
                                          _elmTags.getEnvironment(), getVALRule());
  }

  //NOTE: the copying constructor of successsor MUST properly set _prdArray
  EncoderOfSequencedAC_T(const EncoderOfSequencedAC_T & use_obj)
    : EncoderOfConstructedAC(use_obj), _prdArray(0)
  { }
  //
  void setProducersStorage(ProducersArray & prd_store) { _prdArray = &prd_store; }

public:
  //'Generic sequenced type encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  EncoderOfSequencedAC_T(ProducersArray & prd_store, ElementsArray & elm_store,
                         const ASTagging & eff_tags,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfConstructedAC(prd_store, elm_store, eff_tags, use_rule)
  { }
  //'Generic tagged sequenced type encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  EncoderOfSequencedAC_T(ProducersArray & prd_store, ElementsArray & elm_store,
                         const ASTag & use_tag, ASTagging::Environment_e tag_env,
                         const ASTagging & base_tags,
                         TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfConstructedAC(prd_store, elm_store, use_tag, tag_env, base_tags, use_rule)
  { }
  //
  ~EncoderOfSequencedAC_T()
  { }

  // ----------------------------------------------------------
  // Initialization methods
  // ----------------------------------------------------------
  void setElementTag(const ASTag & elm_tag, ASTagging::Environment_e tag_env) /*throw()*/
  {
    _elmTags.init(elm_tag, tag_env);
  }

  void addValue(const _TArg & use_val) /*throw(std::exception)*/
  {
    _EncoderOfTArg & valEnc = allocElementEncoder();
    valEnc.setValue(use_val);
    initElement(valEnc.getTagging(), *(valEnc.getVALEncoder()));
  }
};

template <
  class _TArg
  //must have tagged & untagged type constructors
  , class _EncoderOfTArg /*: public TypeValueEncoder_T<_TArg>*/ 
  , uint16_t _NumElemsTArg //estimated number of element values
>
class EncoderOfSequenced_T : public EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg> {
private:
  typedef EncoderProducer_T<_EncoderOfTArg> EncoderProducer;

  typedef eyeline::util::LWArray_T<TLVLayoutEncoder, uint16_t, _NumElemsTArg> ElementsStore;
  typedef eyeline::util::LWArray_T<EncoderProducer, uint16_t, _NumElemsTArg> ProducersStore;

  ElementsStore   _elmStore;
  ProducersStore  _prdStore;
protected:
  EncoderOfSequenced_T(const EncoderOfSequenced_T & use_onj)
    : EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg>(use_obj)
  {
    setProducersStorage(_prdStore);
    setElementsStorage(_elmStore);
  }

public:
  //'Generic sequenced type encoder' constructor
  //NOTE: eff_tags must be a complete tagging of type!
  EncoderOfSequenced_T(const ASTagging & eff_tags,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg>(_prdStore, _elmStore, eff_tags, use_rule)
  { }
  //'Generic tagged sequenced type encoder' constructor
  //NOTE: base_tags must be a complete tagging of base type!
  EncoderOfSequenced_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                       const ASTagging & base_tags,
                       TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg>(_prdStore, _elmStore, use_tag, tag_env, base_tags, use_rule)
  { }
  EncoderOfSequenced_T(const EncoderOfSequenced_T & use_obj)
    : EncoderOfSequencedAC_T<_TArg, _EncoderOfTArg>(use_obj)
  {
    setProducersStorage(_prdStore);
    setElementsStorage(_elmStore);
  }
  //
  ~EncoderOfSequenced_T()
  { }
};
} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_SEQUENCED */

