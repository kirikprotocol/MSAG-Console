/* ************************************************************************* *
 * ROS Linked operation Id type encoder.
 * ************************************************************************* */
#ifndef __EYELINE_ROS_PROTO_ENC_LINKEDIDTYPE_HPP
#ident "@(#)$Id$"
#define __EYELINE_ROS_PROTO_ENC_LINKEDIDTYPE_HPP

#include "eyeline/ros/ROSTypes.hpp"

#include "eyeline/asn1/BER/rtenc/EncodeINT.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeNULL.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeChoice.hpp"
#include "eyeline/asn1/BER/rtenc/EncodersChoiceT.hpp"

namespace eyeline {
namespace ros {
namespace proto {
namespace enc {

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
protected:
  using asn1::ber::EncoderOfChoice::setSelection;

  class AltEncoder : public asn1::ber::ChoiceOfEncoders2_T<
                      asn1::ber::EncoderOfINTEGER, asn1::ber::EncoderOfNULL> {
  public:
    Alternative_T<asn1::ber::EncoderOfINTEGER>  present() { return alternative0(); }
    Alternative_T<asn1::ber::EncoderOfNULL>     absent()  { return alternative1(); }

    ConstAlternative_T<asn1::ber::EncoderOfINTEGER> present() const { return alternative0(); }
    ConstAlternative_T<asn1::ber::EncoderOfNULL>    absent()  const { return alternative1(); }
  };

  AltEncoder  _alt;

public:
  static const asn1::ASTag _tagPresent;
  static const asn1::ASTag _tagAbsent;

  explicit RELinkedIdType(asn1::TransferSyntax::Rule_e use_rule
                          = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(use_rule)
  {
    addCanonicalAlternative(_tagPresent, asn1::ASTagging::tagsIMPLICIT);
  }
  RELinkedIdType(const asn1::ASTag & outer_tag, asn1::ASTagging::Environment_e tag_env,
                 asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfChoice(outer_tag, tag_env, use_rule)
  {
    addCanonicalAlternative(_tagPresent, asn1::ASTagging::tagsIMPLICIT);
  }
  //
  ~RELinkedIdType()
  { }

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

