#ifndef __EYELINE_MAP_7F0_PROTO_ERR_ENC_MECALLBARREDPARAM_HPP__
# ident "@(#)$Id$"
# define __EYELINE_MAP_7F0_PROTO_ERR_ENC_MECALLBARREDPARAM_HPP__

# include "util/Exception.hpp"
# include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
# include "eyeline/map/7_15_0/proto/ERR/CallBarredParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/ExtensibleCallBarredParam.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MECallBarringCause.hpp"
# include "eyeline/map/7_15_0/proto/ERR/enc/MEExtensibleCallBarredParam.hpp"

# include "eyeline/util/MaxSizeof.hpp"

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
public:
  explicit MECallBarredParam(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfChoice(use_rule)
  {
    _value.any= NULL;
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    addCanonicalAlternative(asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  }

  MECallBarredParam(const asn1::ASTag& outer_tag,
                    const asn1::ASTagging::Environment_e tag_env,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
  : asn1::ber::EncoderOfChoice(outer_tag, tag_env, use_rule)
  {
    //add canonical alternative tagging (see EncodeChoice.hpp:NOTE.1)
    // is it necessary?
    addCanonicalAlternative(asn1::_tagENUM, asn1::ASTagging::tagsIMPLICIT);
  }

  ~MECallBarredParam()
  {
    cleanup();
  }

  void setValue(const CallBarredParam& value) {
    switch (value.getKind()) {
    case CallBarredParam::KindCallBarringCause:
      setCallBarringCause(*value.getCallBarringCause());
      break;
    case CallBarredParam::KindExtensibleCallBarredParam:
      setExtensibleCallBarredParam(*value.getExtensibleCallBarredParam());
      break;
    default:
      throw smsc::util::Exception("map::7_15_0::proto::ERR::enc::MECallBarredParam::setValue() : invalid value");
    }
  }

  void setCallBarringCause(const CallBarringCause& val) {
    cleanup();
    _value.callBarringCause= new (_memAlloc.buf) MECallBarringCause(getTSRule());
    _value.callBarringCause->setValue(val.value);
    asn1::ber::EncoderOfChoice::setSelection(*_value.callBarringCause);
  }

  void setExtensibleCallBarredParam(const ExtensibleCallBarredParam& value) {
    cleanup();
    _value.any = new (_memAlloc.buf) MEExtensibleCallBarredParam(value, getTSRule());
    asn1::ber::EncoderOfChoice::setSelection(*_value.callBarringCause);
  }

private:
  void cleanup() {
    if (_value.any) {
      _value.any->~TypeEncoderAC();
      _value.any = NULL;
    }
  }

  union {
    void *  aligner;
    uint8_t buf[eyeline::util::MaxSizeOf2_T<MECallBarringCause,
                                            MEExtensibleCallBarredParam>::VALUE];
  } _memAlloc;

  union {
    asn1::ber::TypeEncoderAC * any;
    MECallBarringCause* callBarringCause;
    MEExtensibleCallBarredParam* extensibleCallBarredParam;
  } _value;
};

}}}}

#endif
