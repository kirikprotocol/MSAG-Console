#ifndef __EYELINE_MAP_7F0_PROTO_EXT_DEC_MDPRIVATEEXTENSION_HPP__
#ifndef __GNUC__
# ident "@(#)$Id$"
#endif
# define __EYELINE_MAP_7F0_PROTO_EXT_DEC_MDPRIVATEEXTENSION_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeSeq.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeEOID.hpp"
# include "eyeline/asn1/BER/rtdec/DecoderProducer.hpp"
# include "eyeline/asn1/BER/rtdec/DecodeASType.hpp"
# include "eyeline/map/7_15_0/proto/EXT/PrivateExtension.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace dec {

/*
  Type is defined in IMPLICIT tagging environment as follow:
  PrivateExtension ::= SEQUENCE {
    extId       MAP-EXTENSION.&extensionId ({ExtensionSet}),
    extType     MAP-EXTENSION.&ExtensionType ({ExtensionSet}{@extId}) OPTIONAL
  }
*/
class MDPrivateExtension : public asn1::ber::DecoderOfSequence_T<2,1> {
public:
  explicit MDPrivateExtension(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
  : asn1::ber::DecoderOfSequence_T<2,1>(use_rule),
    _value(NULL), _dExtId(use_rule)
  {
    construct();
  }
  MDPrivateExtension(PrivateExtension& value,
                     asn1::ber::TSGroupBER::Rule_e use_rule = asn1::ber::TSGroupBER::ruleBER)
    : asn1::ber::DecoderOfSequence_T<2,1>(asn1::ber::TSGroupBER::getTSRule(use_rule)),
      _value(&value), _dExtId(asn1::ber::TSGroupBER::getTSRule(use_rule))
  {
    construct();
  }

  void setValue(PrivateExtension& value)
  {
    _value= &value;
  }

protected:
  void construct(void);
  virtual asn1::ber::TypeDecoderAC* prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;

private:
  PrivateExtension* _value;

  asn1::ber::DecoderOfEOID _dExtId;
  asn1::ber::DecoderProducer_T<asn1::ber::DecoderOfASType> _dExtType;
};

}}}}

#endif
