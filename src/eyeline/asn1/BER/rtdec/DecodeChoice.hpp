/* ************************************************************************* *
 * BER Decoder: CHOICE type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_CHOICE
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_CHOICE

#include "eyeline/asn1/BER/rtdec/DecodeConstructed.hpp"
#include "eyeline/asn1/BER/rtdec/ElementDecoderOfCHC.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class DecoderOfChoiceAC : public DecoderOfConstructedAC {
private:
  TaggingOptions        _altTags; //taggings of CHOICE alternatives

protected:
  // -- ************************************************* --
  // -- ValueDecoderIface abstract methods are to implement
  // -- ************************************************* --
  //NOTE: should reset ElementDecoder prior to decoding!
  //NOTE: in case of Untagged CHOICE/ANY/OpenType the identification tag is a
  //      part of value encoding.
  virtual DECResult decodeVAL(const TLParser & tlv_prop, //corresponds to innermost identification tag
                              const uint8_t * use_enc, TSLength max_len,
                              TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                              bool relaxed_rule = false) /*throw(std::exception)*/;

  //NOTE: copying constructor of successsor MUST properly set _elDec
  //      by calling setElementDecoder()
  explicit DecoderOfChoiceAC(const DecoderOfChoiceAC & use_obj)
    : DecoderOfConstructedAC(use_obj), _altTags(use_obj._altTags)
  {
    TypeTagging::setOptions(_altTags);
  }

  explicit DecoderOfChoiceAC(ElementDecoderByTagAC & use_eldec,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfConstructedAC(use_eldec, use_rule)
  {
    TypeTagging::setOptions(_altTags);
    //NOTE.1: in case of untagged CHOICE, tagging of canonical alternative
    //        MUST BE added by setAlternative() to alternative's tagging
    //        options in successor's constructor in order to support CER !!!
  }
  // NOTE: eff_tags is a complete effective tagging of type!
  DecoderOfChoiceAC(ElementDecoderByTagAC & use_eldec, const ASTagging & eff_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfConstructedAC(use_eldec, eff_tags, use_rule)
  {
    TypeTagging::setOptions(_altTags);
  }

  //Sets tagged alternative of some type
  void setAlternative(uint16_t unique_idx, const ASTag & fld_tag, ASTagging::Environment_e fld_env)
    /*throw(std::exception)*/;

  //Sets untagged alternative of ordinary type
  void setAlternative(uint16_t unique_idx, const ASTag & fld_tag) /*throw(std::exception)*/;

  //Sets untagged alternative of ANY/Opentype
  void setAlternative(uint16_t unique_idx) /* throw(std::exception)*/;

  //Sets untagged alternative of untagged CHOICE type
  void setAlternative(uint16_t unique_idx, const TaggingOptions & use_tag_opts) /*throw(std::exception)*/;

  //Sets alternative for unknown extension additions entry
  void setUnkExtension(uint16_t unique_idx) /* throw(std::exception)*/;

public:
  virtual ~DecoderOfChoiceAC()
  { }
};


template <uint16_t _SizeTArg>
class DecoderOfChoice_T : public DecoderOfChoiceAC {
private:
  CHCElementDecoder_T<_SizeTArg>  _chcDec;

protected:
  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //Allocates alternative data structure and initializes associated TypeDecoderAC
  //virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx)
  //  /*throw(throw(std::exception)) */ = 0;
  //Perfoms actions finalizing alternative decoding
  //virtual void markDecodedOptional(uint16_t unique_idx)
  //  /*throw(throw(std::exception)) */ { return; }

public:
  // constructor for untagged CHOICE
  explicit DecoderOfChoice_T(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfChoiceAC(_chcDec, use_rule)
  {
    //NOTE.1: in case of untagged CHOICE, tagging of canonical alternative
    //        MUST BE added by setAlternative() to alternative's tagging
    //        options in successor's constructor in order to support CER !!!
  }
  // constructor for tagged CHOICE
  DecoderOfChoice_T(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : DecoderOfChoiceAC(_chcDec, ASTagging(use_tag, tag_env), use_rule)
  { }
  //
  DecoderOfChoice_T(const DecoderOfChoice_T & use_obj)
    : DecoderOfChoiceAC(use_obj), _chcDec(use_obj._chcDec)
  {
    setElementDecoder(_chcDec);
  }
  ~DecoderOfChoice_T()
  { }
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_CHOICE */

