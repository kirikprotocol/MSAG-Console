#ifndef __EYELINE_MAP_7F0_PROTO_EXT_DEC_MDEXTENSIONCONTAINER_HPP__
# define __EYELINE_MAP_7F0_PROTO_EXT_DEC_MDEXTENSIONCONTAINER_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeUExt.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"

# include "eyeline/map/7_15_0/proto/EXT/ExtensionContainer.hpp"
# include "eyeline/map/7_15_0/proto/EXT/dec/MDPCS_Extensions.hpp"
# include "eyeline/map/7_15_0/proto/EXT/dec/MDPrivateExtensionList.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace dec {

/*
 ExtensionContainer ::= SEQUENCE {
    privateExtensionList    [0] PrivateExtensionList OPTIONAL,
    pcs-Extensions          [1] PCS-Extensions       OPTIONAL,
    ...
 }
*/
class MDExtensionContainer : public asn1::ber::DecoderOfSequence_T<3,1> {
public:
  explicit MDExtensionContainer(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<3,1>(use_rule),
    _value(NULL)
  {
    construct();
  }

  explicit MDExtensionContainer(ExtensionContainer& value,
                                asn1::ber::TSGroupBER::Rule_e use_rule = asn1::ber::TSGroupBER::ruleBER)
  : asn1::ber::DecoderOfSequence_T<3,1>(asn1::ber::TSGroupBER::getTSRule(use_rule)),
    _value(&value)
  {
    construct();
  }

  void setValue(ExtensionContainer& value)
  {
    _value = &value;
  }

protected:
  void construct(void);
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  virtual void markDecodedOptional(uint16_t unique_idx) {}

private:
  ExtensionContainer* _value;

  asn1::ber::DecoderProducer_T<MDPrivateExtensionList> _dPrvtExtLst;
  asn1::ber::DecoderProducer_T<MDPCS_Extensions> _dPcsExts;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfUExtension> _uext;

  static const asn1::ASTag _tag_PrvtExtLst;
  static const asn1::ASTag _tag_PCS_Ext;
};

}}}}

#endif /* MDEXTENSIONCONTAINER_HPP_ */
