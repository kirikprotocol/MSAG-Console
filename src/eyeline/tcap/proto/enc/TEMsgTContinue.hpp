/* ************************************************************************* *
 * TCAP Continue Message Encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_TCONT_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_TCONT_HPP

#include "eyeline/tcap//TComponentDefs.hpp"
#include "eyeline/tcap/proto/enc/TETransactionId.hpp"
#include "eyeline/tcap/proto/enc/TEDialoguePortion.hpp"
#include "eyeline/tcap/proto/enc/TEComponentPortion.hpp"

#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

/* Continue is defined in IMPLICIT tagging environment as follow:
Continue ::= [APPLICATION 5] SEQUENCE {
  otid             OrigTransactionID,
  dtid             DestTransactionID,
  dialoguePortion  DialoguePortion OPTIONAL,
  components       ComponentPortion OPTIONAL
} */
class TETContinue : public asn1::ber::EncoderOfStructure_T<4> {
protected:
  TEOrigTransactionId           _trIdSrc;
  TEDestTransactionId           _trIdDst;
  //NOTE: only optional AARE_APdu is allowed
  asn1::ber::EncoderProducer_T<TEDialoguePortionStructured> _partDlg;
  asn1::ber::EncoderProducer_T<TEComponentPortion>          _partComp;

  TEDialoguePortionStructured * initDlgPortion(void);
  TEComponentPortion * initCompPortion(void);

public:
  static const asn1::ASTagging _typeTags;

  TETContinue(uint32_t local_tr_id, uint32_t remote_tr_id,
              asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<4>(_typeTags, use_rule)
    , _trIdSrc(local_tr_id, use_rule), _trIdDst(remote_tr_id, use_rule)
  {
    asn1::ber::EncoderOfStructure_T<4>::setField(0, _trIdSrc);
    asn1::ber::EncoderOfStructure_T<4>::setField(1, _trIdDst);
  }
  ~TETContinue()
  { }

  // --------------------------------------------------------------------
  //NOTE: calling DialoguePortion initialization methods is allowed only
  //      in case of TEnd is an immediate response to TBegin
  // --------------------------------------------------------------------
  //Creates and initializes AARE_APdu (sets ApplicationContext)
  TEAPduAARE * initDlgResponse(const asn1::EncodedOID & use_app_ctx)
  {
    return initDlgPortion()->initPduAARE(use_app_ctx);
  }

  //Creates and appends new component (ROS Pdu) encoder
  void addComponent(const ros::ROSPdu & ros_pdu)
  {
    initCompPortion()->addElementValue(ros_pdu);
  }

  void addComponents(const tcap::TComponentsPtrList & comp_list)
  {
    initCompPortion()->setValue(comp_list);
  }

  void clearCompPortion(void);
  void clearDlgPortion(void);
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_TCONT_HPP */

