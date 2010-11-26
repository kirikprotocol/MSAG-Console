#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MESYSTEMFAILUREPARAM_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MESYSTEMFAILUREPARAM_HPP__

# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/SystemFailureParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleSystemFailureParam.hpp"

# include "eyeline/map/7_15_0/proto/common/enc/MENetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MEExtensibleSystemFailureParam.hpp"

# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
# include "eyeline/asn1/BER/rtenc/EncodersChoiceT.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace enc {

/* Type is defined in IMPLICIT tagging environment as follow:
   SystemFailureParam ::= CHOICE {
        networkResource NetworkResource,
            -- networkResource must not be used in version 3
        extensibleSystemFailureParam    ExtensibleSystemFailureParam
            -- extensibleSystemFailureParam must not be used in version <3
   }
*/
class MESystemFailureParam : public asn1::ber::EncoderOfChoice {
private:
  asn1::ber::EncoderOfChoice::setSelection;

protected:
  class AltEncoder : public asn1::ber::ChoiceOfEncoders2_T<
                        common::enc::MENetworkResource,
                        MEExtensibleSystemFailureParam > {
  public:
    Alternative_T<common::enc::MENetworkResource>   networkResource()  { return alternative0(); }
    Alternative_T<MEExtensibleSystemFailureParam>   extensibleSystemFailureParam()  { return alternative1(); }

    ConstAlternative_T<common::enc::MENetworkResource>  networkResource()  const { return alternative0(); }
    ConstAlternative_T<MEExtensibleSystemFailureParam>  extensibleSystemFailureParam()  const { return alternative1(); }
  };

  AltEncoder  _altEnc;

public:
  explicit MESystemFailureParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  }
  //
  MESystemFailureParam(const asn1::ASTag& outer_tag,
                       const asn1::ASTagging::Environment_e tag_env,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(outer_tag, tag_env, use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  }
  //
  ~MESystemFailureParam()
  { }

  void setValue(const SystemFailureParam& value);
  void setNetworkResource(const common::NetworkResource& use_val);
  void setExtensibleSystemFailureParam(const ExtensibleSystemFailureParam& use_val);
};

}}}}

#endif
