#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDCALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDCALLBARREDPARAM_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
# include "eyeline/asn1/BER/rtdec/DecodersChoiceT.hpp"
# include "eyeline/map/7_15_0/proto/ERR/CallBarredParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/dec/MDCallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/ERR/dec/MDExtensibleCallBarredParam.hpp"

namespace eyeline {
namespace map {
namespace err {
namespace dec {

/* Type is defined in IMPLICIT tagging environment as follow:
 CallBarredParam ::= CHOICE {
        callBarringCause        CallBarringCause,
        -- call BarringCause must not be used in version 3 and higher
        extensibleCallBarredParam       ExtensibleCallBarredParam
        -- extensibleCallBarredParam must not be used in version <3
 }
*/
class MDCallBarredParam : public asn1::ber::DecoderOfChoice_T<2> {
public:
  explicit MDCallBarredParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfChoice_T<2>(use_rule), _value(NULL)
  {
    construct();
  }

  MDCallBarredParam(const asn1::ASTag& outer_tag,
                    const asn1::ASTagging::Environment_e tag_env,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfChoice_T<2>(outer_tag, tag_env, use_rule), _value(NULL)
  {
    construct();
  }

  void setValue(CallBarredParam& value) {
    _value= &value;
    _chcDec.reset();
  }

protected:
  void construct();
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  class AltDecoder : public asn1::ber::ChoiceOfDecoders2_T<MDCallBarringCause,
                                                           MDExtensibleCallBarredParam> {
  public:
    Alternative_T<MDCallBarringCause>           callBarringCause()  { return alternative0(); }
    Alternative_T<MDExtensibleCallBarredParam>  extensibleCallBarredParam()  { return alternative1(); }

    ConstAlternative_T<MDCallBarringCause>            callBarringCause()  const { return alternative0(); }
    ConstAlternative_T<MDExtensibleCallBarredParam>   extensibleCallBarredParam()  const { return alternative1(); }
  };

  AltDecoder  _altDec;
  CallBarredParam* _value;
};

}}}}

#endif
