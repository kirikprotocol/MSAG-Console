/* ************************************************************************* *
 * ROS Linked operation Id type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_LINKEDIDTYPE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_ROS_PROTO_DEC_LINKEDIDTYPE_HPP

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeNULL.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"
#include "eyeline/asn1/BER/rtdec/DecodersChoiceT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace dec {

using eyeline::asn1::ber::TSGroupBER;

/* LinkedIdType is defined in IMPLICIT tagging environment as follow:
  LinkedIdType ::=  CHOICE {
      present  [0] IMPLICIT InvokeIdType,
      absent   [1] IMPLICIT NULL
  }

  NOTE: In case of LinkedIdType used only with 'present' alternative,
        this definition may be optimized as follow:

  LinkedIdType ::= [0] IMPLICIT InvokeIdType
*/
class RDLinkedIdType : public asn1::ber::DecoderOfChoice_T<2> {
protected:
  class AltDecoder : public asn1::ber::ChoiceOfDecoders2_T<
                      asn1::ber::DecoderOfINTEGER, asn1::ber::DecoderOfNULL> {
  public:
    Alternative_T<asn1::ber::DecoderOfINTEGER> present() { return alternative0(); }
    Alternative_T<asn1::ber::DecoderOfNULL>    absent()  { return alternative1(); }

    ConstAlternative_T<asn1::ber::DecoderOfINTEGER>  present() const { return alternative0(); }
    ConstAlternative_T<asn1::ber::DecoderOfNULL>     absent()  const { return alternative1(); }
  };

  AltDecoder      _alt;
  ros::LinkedId * _dVal;

  //Initializes ElementDecoder for this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //If necessary, allocates alternative and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/;
  //Perfoms actions finalizing alternative decoding
  virtual void markDecodedAlternative(uint16_t unique_idx) /*throw(throw(std::exception)) */;

public:
  static const asn1::ASTag _tagPresent;
  static const asn1::ASTag _tagAbsent;

  class TaggingOptions : public asn1::ber::TaggingOptions {
  public:
    TaggingOptions() : asn1::ber::TaggingOptions()
    {
      addTagging(_tagPresent, asn1::ASTagging::tagsIMPLICIT);
      addTagging(_tagAbsent, asn1::ASTagging::tagsIMPLICIT);
    }
  };

  static const TaggingOptions _tagOptions;

  explicit RDLinkedIdType(asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<2>(use_rule)
    , _dVal(0)
  {
    construct();
  }
  RDLinkedIdType(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleBER)
    : asn1::ber::DecoderOfChoice_T<2>(outer_tag, tag_env, use_rule)
    , _dVal(0)
  {
    construct();
  }
  //
  ~RDLinkedIdType()
  { }

  void setValue(ros::LinkedId & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _chcDec.reset();
  }
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_LINKEDIDTYPE_HPP */

