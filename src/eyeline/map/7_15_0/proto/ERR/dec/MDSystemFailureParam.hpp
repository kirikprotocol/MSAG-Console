#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDSYSTEMFAILUREPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDSYSTEMFAILUREPARAM_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
# include "eyeline/asn1/BER/rtdec/DecodersChoiceT.hpp"
# include "eyeline/map/7_15_0/proto/ERR/SystemFailureParam.hpp"
# include "eyeline/map/7_15_0/proto/common/dec/MDNetworkResource.hpp"
# include "eyeline/map/7_15_0/proto/ERR/dec/MDExtensibleSystemFailureParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/* Type is defined in IMPLICIT tagging environment as follow:
   SystemFailureParam ::= CHOICE {
        networkResource NetworkResource,
            -- networkResource must not be used in version 3
        extensibleSystemFailureParam    ExtensibleSystemFailureParam
            -- extensibleSystemFailureParam must not be used in version <3
   }
*/
class MDSystemFailureParam : public asn1::ber::DecoderOfChoice_T<2> {
public:
  explicit MDSystemFailureParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfChoice_T<2>(use_rule), _value(NULL)
  {
    construct();
  }

  MDSystemFailureParam(const asn1::ASTag& outer_tag,
                       const asn1::ASTagging::Environment_e tag_env,
                       asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfChoice_T<2>(outer_tag, tag_env, use_rule), _value(NULL)
  {
    construct();
  }

  void setValue(SystemFailureParam& value) {
    _value= &value;
    _chcDec.reset();
  }

protected:
  void construct();
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  class AltDecoder : public asn1::ber::ChoiceOfDecoders2_T<common::dec::MDNetworkResource,
                                                           MDExtensibleSystemFailureParam> {
  public:
    Alternative_T<common::dec::MDNetworkResource>  networkResource() { return alternative0(); }
    Alternative_T<MDExtensibleSystemFailureParam>  extensibleSystemFailureParam() { return alternative1(); }

    ConstAlternative_T<common::dec::MDNetworkResource>   networkResource()  const { return alternative0(); }
    ConstAlternative_T<MDExtensibleSystemFailureParam>   extensibleSystemFailureParam()  const { return alternative1(); }
  };

  AltDecoder  _altDec;
  SystemFailureParam* _value;
};

}}}}

#endif
