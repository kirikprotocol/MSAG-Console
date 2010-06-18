/* ************************************************************************* *
 * BER Encoder: CHOICE type encoder.
 * ************************************************************************* */
#ifndef __ASN1_BER_ENCODER_CHOICE
#ident "@(#)$Id$"
#define __ASN1_BER_ENCODER_CHOICE

#include "eyeline/asn1/BER/rtenc/TLVEncoder.hpp"

namespace eyeline {
namespace asn1 {
namespace ber {


class EncoderOfChoice : public TypeEncoderAC {
private:
  TaggingOptions  _altTags; //taggings of CHOICE alternatives

  using TypeEncoderAC::init;
  //
  void setAlternative(const ASTagging & use_tags, ValueEncoderIface * val_enc);

protected:
  void addCanonicalAlternative(const ASTag & use_tag, ASTagging::Environment_e tag_env)
  {
    _altTags.addTagging(use_tag, tag_env);
  }
  //
  void addCanonicalAlternative(const ASTagging & use_tags)
  {
    _altTags.addTagging(use_tags);
  }
  //
  void addCanonicalAlternative(const TaggingOptions & use_opts)
  {
    _altTags.addOptions(use_opts);
  }

  //
  EncoderOfChoice(const EncoderOfChoice & use_obj)
    : TypeEncoderAC(use_obj), _altTags(use_obj._altTags)
  {
    TypeTagging::setOptions(_altTags);
  }
  // constructor for tagged type referencing CHOICE
  // NOTE: eff_tags must be a complete tagging of type!
  EncoderOfChoice(const ASTagging & eff_tags,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(eff_tags, _altTags, use_rule)
  { }

public:
  // constructor for untagged CHOICE
  EncoderOfChoice(TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(_altTags, use_rule)
  {
    //NOTE.1: in case of untagged CHOICE, tagging of canonical alternative
    //        MUST BE added by addCanonicalAlternative() to alternative's tagging
    //        options in successor's constructor in order to support CER !!!
  }
  // constructor for tagged CHOICE
  EncoderOfChoice(const ASTag & use_tag, ASTagging::Environment_e tag_env,
                  TransferSyntax::Rule_e use_rule = TransferSyntax::ruleDER)
    : TypeEncoderAC(use_tag, tag_env, _altTags, use_rule)
  { }
  //
  virtual ~EncoderOfChoice()
  { }

  //value of CHOICE is an untagged alternative
  void setSelection(TypeEncoderAC & type_enc)/*throw(std::exception)*/;
  //value of CHOICE is a tagged alternative
  void setSelection(TypeEncoderAC & type_enc,
                const ASTag & fld_tag, ASTagging::Environment_e fld_env)
                /*throw(std::exception)*/;
};

} //ber
} //asn1
} //eyeline

#endif /* __ASN1_BER_ENCODER_CHOICE */

