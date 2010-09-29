/* ************************************************************************* *
 * TCAP Message Encoder: encoder of APDUs field UserInformation.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_ENC_USERINFO_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_ENC_USERINFO_HPP

#include "eyeline/tcap/TDlgUserInfo.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeExternal.hpp"
#include "eyeline/asn1/BER/rtenc/EncodeSequenced.hpp"


namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

using eyeline::asn1::ber::TSGroupBER;

/* UserInformation is defined in EXPLICIT tagging environment as follow:
  UserInformation ::= [30] IMPLICIT SEQUENCE OF EXTERNAL
*/
static const uint8_t _TMsgDfltNumOfUserInfos = 1;

class TEUserInformation : public
  asn1::ber::EncoderOfSequenced_T<asn1::ASExternal, asn1::ber::EncoderOfExternal,
                                  _TMsgDfltNumOfUserInfos> {
public:
  static const asn1::ASTagging _typeTags;

  explicit TEUserInformation(TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfSequenced_T<asn1::ASExternal, asn1::ber::EncoderOfExternal,
                                      _TMsgDfltNumOfUserInfos
                                      >(_typeTags, TSGroupBER::getTSRule(use_rule))
  { }
  ~TEUserInformation()
  { }

  //
  //EncoderOfSequenced_T<> provides method
  //void addValue(const asn1::ASExternal & use_val) /*throw(std::exception)*/;

  void addValuesList(const tcap::TDlgUserInfoPtrList & ui_list) /*throw(std::exception)*/;
};

}}}}

#endif /* __EYELINE_TCAP_PROTO_ENC_USERINFO_HPP */

