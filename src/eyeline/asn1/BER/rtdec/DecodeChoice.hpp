/* ************************************************************************* *
 * BER Decoder: CHOICE type decoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_DECODER_CHOICE
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __ASN1_BER_DECODER_CHOICE

#include "eyeline/asn1/BER/rtdec/ElementDecoderOfCHC.hpp"
#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {

class DecoderOfChoiceAC : public TypeValueDecoderAC {
private:
  TaggingOptions        _altTags; //taggings of CHOICE alternatives
  CHCElementDecoderAC * _elDec;   //actually it's just a reference to a successor
                                  //member, so its copying constructor MUST properly
                                  //that pointer.

  DECResult decodeElement(const uint8_t * use_enc, TSLength max_len,
                          bool relaxed_rule) /*throw(std::exception)*/;
protected:
  // -- ************************************************* --
  // -- ValueDecoderIface abstract methods are to implement
  // -- ************************************************* --
  virtual DECResult decodeVAL(const TLVProperty * val_prop,
                      const uint8_t * use_enc, TSLength max_len,
                      TSGroupBER::Rule_e use_rule = TSGroupBER::ruleBER,
                      bool relaxed_rule = false)
    /*throw(std::exception)*/;

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //Allocates alternative data structure and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx)
    /*throw(throw(std::exception)) */ = 0;
  //Perfoms actions finalizing alternative decoding
  virtual void markDecodedAlternative(uint16_t unique_idx)
    /*throw(throw(std::exception)) */ { return; }

  void setElementDecoder(CHCElementDecoderAC & elm_dec) { _elDec = &elm_dec; }

  //NOTE: copying constructor of successsor MUST properly set _elDec
  //      by calling setElementDecoder()
  DecoderOfChoiceAC(const DecoderOfChoiceAC & use_obj)
    : TypeValueDecoderAC(use_obj), _elDec(0)
  {
    setOptions(_altTags);
  }

  explicit DecoderOfChoiceAC(CHCElementDecoderAC & use_eldec,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(&_altTags, use_rule), _elDec(&use_eldec)
  {
    //NOTE.1: in case of untagged CHOICE, tagging of canonical alternative
    //        MUST BE added by setAlternative() to alternative's tagging
    //        options in successor's constructor in order to support CER !!!
  }
  // NOTE: eff_tags is a complete effective tagging of type!
  DecoderOfChoiceAC(CHCElementDecoderAC & use_eldec, const ASTagging & eff_tags,
                    TransferSyntax::Rule_e use_rule = TransferSyntax::ruleBER)
    : TypeValueDecoderAC(eff_tags, _altTags, use_rule), _elDec(&use_eldec)
  { }

  //Sets tagged alternative of some type
  void setAlternative(uint16_t unique_idx, const ASTag & fld_tag, ASTagging::Environment_e fld_env)
    /*throw(std::exception)*/
  {
    if (!isTagged()) //untagged CHOICE
      _altTags.addTagging(fld_tag, fld_env);
    _elDec->setAlternative(unique_idx, fld_tag, fld_env, EDAlternative::altOPTIONAL);
  }
  //Sets untagged alternative of ordinary type
  void setAlternative(uint16_t unique_idx, const ASTag & fld_tag)
    /*throw(std::exception)*/
  {
    if (!isTagged()) //untagged CHOICE
      _altTags.addTagging(fld_tag, ASTagging::tagsIMPLICIT);
    _elDec->setAlternative(unique_idx, fld_tag, EDAlternative::altOPTIONAL);
  }
  //Sets untagged alternative of ANY/Opentype
  void setAlternative(uint16_t unique_idx)
    /* throw(std::exception)*/
  {
    if (!isTagged()) //untagged CHOICE
      _altTags.addTagging(asn1::_tagANYTYPE, ASTagging::tagsIMPLICIT);
    _elDec->setAlternative(unique_idx, EDAlternative::altOPTIONAL);
  }
  //Sets untagged alternative of untagged CHOICE type
  void setAlternative(uint16_t unique_idx, const TaggingOptions & use_tag_opts)
    /*throw(std::exception)*/
  {
    if (!isTagged()) //untagged CHOICE
      _altTags.addOptions(use_tag_opts);
    _elDec->setAlternative(unique_idx, use_tag_opts, EDAlternative::altOPTIONAL);
  }
  //Sets alternative for unknown extension additions entry
  void setUnkExtension(uint16_t unique_idx) /* throw(std::exception)*/
  {
    if (!isTagged()) //untagged CHOICE
      _altTags.addTagging(asn1::_tagUNI0, ASTagging::tagsIMPLICIT);
    _elDec->setUnkExtension(unique_idx);
  }

public:
  virtual ~DecoderOfChoiceAC()
  { }
};


template <uint16_t _SizeTArg>
class DecoderOfChoice_T : public DecoderOfChoiceAC {
protected:
  CHCElementDecoder_T<_SizeTArg>  _chcDec;

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //Allocates alternative data structure and initializes associated TypeDecoderAC
  //virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx)
  //  /*throw(throw(std::exception)) */ = 0;
  //Perfoms actions finalizing alternative decoding
  //virtual void markDecodedAlternative(uint16_t unique_idx)
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

