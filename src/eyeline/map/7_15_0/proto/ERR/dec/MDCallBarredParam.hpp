#ifndef __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDCALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_DEC_MDCALLBARREDPARAM_HPP__

# include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"
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
  : asn1::ber::DecoderOfChoice_T<2>(use_rule),
    _value(NULL)
  {
    _pDec._any = 0;
    construct();
  }

  explicit MDCallBarredParam(ExtensibleCallBarredParam& value,
                             asn1::ber::TSGroupBER::Rule_e use_rule = asn1::ber::TSGroupBER::ruleBER)
  : asn1::ber::DecoderOfChoice_T<2>(asn1::ber::TSGroupBER::getTSRule(use_rule)),
    _value(&value)
  {
    _pDec._any = 0;
    construct();
  }

  MDCallBarredParam(const asn1::ASTag& outer_tag,
                    const asn1::ASTagging::Environment_e tag_env,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::DecoderOfChoice_T<2>(outer_tag, tag_env, use_rule)
  {
    _pDec._any = 0;
    construct();
  }

  ~MDCallBarredParam() {
    cleanup();
  }

  void setValue(ExtensibleCallBarredParam& value) {
    _value= &value;
    _chcDec.reset();
  }

protected:
  void construct();
  virtual asn1::ber::TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception) */;
  virtual void markDecodedOptional(uint16_t unique_idx) {}

private:
  ExtensibleCallBarredParam* _value;
  union {
    asn1::ber::TypeDecoderAC * _any;
    MDCallBarringCause*   _callBarringCause;
    MDExtensibleCallBarredParam*  _extCallBarredParam;
  } _pDec;

  union {
    void *  _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf2_T<MDCallBarringCause,
                                             MDExtensibleCallBarredParam>::VALUE];
  } _memAlt;

  void cleanup(void)
  {
    if (_pDec._any) {
      _pDec._any->~TypeDecoderAC();
      _pDec._any = NULL;
    }
  }
};

}}}}

#endif
