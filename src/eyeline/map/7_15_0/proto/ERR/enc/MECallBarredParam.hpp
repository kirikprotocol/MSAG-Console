#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MECALLBARREDPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MECALLBARREDPARAM_HPP__

# include "eyeline/map/7_15_0/proto/ERR/CallBarredParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"

# include "eyeline/map/7_15_0/proto/ERR/enc/MECallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MEExtensibleCallBarredParam.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
# include "eyeline/asn1/BER/rtenc/EncodersChoiceT.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
 CallBarredParam ::= CHOICE {
      callBarringCause        CallBarringCause,
            -- call BarringCause must not be used in version 3 and higher
      extensibleCallBarredParam       ExtensibleCallBarredParam
          -- extensibleCallBarredParam must not be used in version <3
 }
*/
class MECallBarredParam : public asn1::ber::EncoderOfChoice {
private:
  asn1::ber::EncoderOfChoice::setSelection;

protected:
  class AltEncoder : public asn1::ber::ChoiceOfEncoders2_T<
                        MECallBarringCause, MEExtensibleCallBarredParam> {
  public:
    Alternative_T<MECallBarringCause>           callBarringCause()  { return alternative0(); }
    Alternative_T<MEExtensibleCallBarredParam>  extensibleCallBarredParam()  { return alternative1(); }

    ConstAlternative_T<MECallBarringCause>            callBarringCause()  const { return alternative0(); }
    ConstAlternative_T<MEExtensibleCallBarredParam>   extensibleCallBarredParam()  const { return alternative1(); }
  };

  AltEncoder  _altEnc;

public:
  explicit MECallBarredParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  }
  //
  MECallBarredParam(const asn1::ASTag& outer_tag,
                    const asn1::ASTagging::Environment_e tag_env,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(outer_tag, tag_env, use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  }
  //
  ~MECallBarredParam()
  { }

  void setValue(const CallBarredParam& value);
  void setCallBarringCause(const CallBarringCause& use_val);
  void setExtensibleCallBarredParam(const ExtensibleCallBarredParam& use_val);
};

}}}}

#endif
