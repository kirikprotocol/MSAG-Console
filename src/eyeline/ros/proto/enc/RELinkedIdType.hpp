/* ************************************************************************* *
 * ROS Linked operation Id type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_LINKEDIDTYPE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_LINKEDIDTYPE_HPP

#include "eyeline/util/MaxSizeof.hpp"

#include "eyeline/ros/ROSTypes.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

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

class RELinkedIdType : public asn1::ber::EncoderOfChoice {
private:
  union {
    void * _aligner;
    uint8_t _buf[eyeline::util::MaxSizeOf2_T<asn1::ber::EncoderOfINTEGER,
                                            asn1::ber::EncoderOfNULL>::VALUE];
  } _memAlt;

protected:
  union {
    asn1::ber::TypeEncoderAC *    _none;
    asn1::ber::EncoderOfINTEGER * _present;
    asn1::ber::EncoderOfNULL *    _absent;
  } _alt;

  void resetAlt(void);

public:
  static const asn1::ASTagging _tagsPresent;
  static const asn1::ASTagging _tagsAbsent;

  explicit RELinkedIdType(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfChoice(TSGroupBER::getTSRule(use_rule))
  {
    _memAlt._aligner = 0;
    _alt._none = NULL;
    addCanonicalAlternative(_tagsPresent);
  }
  RELinkedIdType(ros::InvokeId inv_id,
                 TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfChoice(TSGroupBER::getTSRule(use_rule))
  {
    _memAlt._aligner = 0;
    _alt._none = NULL;
    addCanonicalAlternative(_tagsPresent);
    setIdLinked(inv_id);
  }
  //
  ~RELinkedIdType()
  {
    resetAlt();
  }

  void setIdLinked(ros::InvokeId inv_id) /*throw(std::exception)*/;

  void setIdAbsent(void) /*throw(std::exception)*/;

  void setValue(const ros::LinkedId & use_val) /*throw(std::exception)*/
  {
    if (!use_val._present)
      setIdAbsent();
    else
      setIdLinked(use_val._invId);
  }
};


}}}}

#endif /* __EYELINE_ROS_PROTO_ENC_LINKEDIDTYPE_HPP */

