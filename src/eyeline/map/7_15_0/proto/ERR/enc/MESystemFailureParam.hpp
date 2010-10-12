#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MESYSTEMFAILUREPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MESYSTEMFAILUREPARAM_HPP__

# include "util/Exception.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
# include "eyeline/map/7_15_0/proto/common/NetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/common/enc/MENetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/SystemFailureParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleSystemFailureParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MEExtensibleSystemFailureParam.hpp"

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
public:
  explicit MESystemFailureParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfChoice(use_rule)
  {
    _value.any= NULL;
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  }

  MESystemFailureParam(const asn1::ASTag& outer_tag,
                       const asn1::ASTagging::Environment_e tag_env,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfChoice(outer_tag, tag_env, use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    // is it necessary?
    addCanonicalAlternative(asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  }

  ~MESystemFailureParam()
  {
    cleanup();
  }

  void setValue(const SystemFailureParam& value);
  void setNetworkResource(const common::NetworkResource& val);
  void setExtensibleSystemFailureParam(const ExtensibleSystemFailureParam& value);

private:
  void cleanup() {
    if (_value.any) {
      _value.any->~TypeEncoderAC();
      _value.any = NULL;
    }
  }

  union {
    void *  aligner;
    uint8_t buf[eyeline::util::MaxSizeOf2_T<common::enc::MENetworkResource,
                                            MEExtensibleSystemFailureParam>::VALUE];
  } _memAlloc;

  union {
    asn1::ber::TypeEncoderAC* any;
    common::enc::MENetworkResource* networkResource;
    MEExtensibleSystemFailureParam* extensibleSystemFailureParam;
  } _value;
};

}}}}

#endif
