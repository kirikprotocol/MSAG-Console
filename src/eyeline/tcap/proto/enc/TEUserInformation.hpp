/* ************************************************************************* *
 * TCAP Message Encoder: encoder of APDUs field UserInformation.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_USERINFO_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_USERINFO_HPP

#include "eyeline/tcap/TDlgUserInfo.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeExternal.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* UserInformation is defined in EXPLICIT tagging environment as follow:
  UserInformation ::= [30] IMPLICIT SEQUENCE OF EXTERNAL
*/
//static const uint8_t _dfltUI_ElementsNum = 1;
class TEUserInformation : public asn1::ber::EncoderOfStructure_T<1> {
private:
  using asn1::ber::EncoderOfStructure_T<1>::addField;
  using asn1::ber::EncoderOfStructure_T<1>::setField;

protected:
  typedef eyeline::util::LWArray_T<asn1::ber::EncoderOfExternal, uint8_t, 1> UIArray;

  UIArray  _uiArray;
  
public:
  static const asn1::ASTagging _typeTags;

  TEUserInformation(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfStructure_T<1>(_typeTags, TSGroupBER::getTSRule(use_rule))
  { }
  ~TEUserInformation()
  { }

  asn1::ber::EncoderOfExternal * addUIValue(const asn1::ASExternal & val_ext);

  void addUIList(const tcap::TDlgUserInfoPtrList & ui_list);
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_USERINFO_HPP */

