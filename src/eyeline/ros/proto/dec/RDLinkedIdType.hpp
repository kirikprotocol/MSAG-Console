/* ************************************************************************* *
 * ROS Linked operation Id type decoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_DEC_LINKEDIDTYPE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_DEC_LINKEDIDTYPE_HPP

#include "eyeline/util/MaxSizeof.hpp"

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeINT.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeNULL.hpp"
#include "eyeline/asn1/BER/rtdec/DecodeChoice.hpp"

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
private:
  union {
    void *  _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf2_T<asn1::ber::DecoderOfINTEGER,
                                            asn1::ber::DecoderOfNULL>::VALUE];
  } _memAlt;

protected:
  ros::LinkedId * _dVal;
  union {
    asn1::ber::TypeDecoderAC *    _none;
    asn1::ber::DecoderOfINTEGER * _present;
    asn1::ber::DecoderOfNULL *    _absent;
  } _alt;

  void resetAlt(void);
  //Initializes ElementDecoder for this type
  void construct(void);

  // ----------------------------------------
  // -- DecoderOfChoiceAC interface methods
  // ----------------------------------------
  //If necessary, allocates alternative and initializes associated TypeDecoderAC
  virtual TypeDecoderAC * prepareAlternative(uint16_t unique_idx) /*throw(std::exception)*/;

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

  explicit RDLinkedIdType(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfChoice_T<2>(TSGroupBER::getTSRule(use_rule))
    , _dVal(0)
  {
    _memAlt._aligner = 0;
    construct();
  }
  RDLinkedIdType(ros::LinkedId & use_val,
                TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::DecoderOfChoice_T<2>(TSGroupBER::getTSRule(use_rule))
    , _dVal(&use_val)
  {
    _memAlt._aligner = 0;
    construct();
  }
  //
  ~RDLinkedIdType()
  {
    resetAlt();
  }

  void setValue(ros::LinkedId & use_val) /*throw(std::exception)*/
  {
    _dVal = &use_val;
    _chcDec.reset();
  }
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_LINKEDIDTYPE_HPP */

