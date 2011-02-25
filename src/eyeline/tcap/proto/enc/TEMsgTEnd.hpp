/* ************************************************************************* *
 * TCAP End Message Encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_TEND_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_TCAP_PROTO_TEND_HPP

#include "eyeline/tcap//TComponentDefs.hpp"
#include "eyeline/tcap/proto/enc/TETransactionId.hpp"
#include "eyeline/tcap/proto/enc/TEDialoguePortion.hpp"
#include "eyeline/tcap/proto/enc/TEComponentPortion.hpp"

#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

/* End is defined in IMPLICIT tagging environment as follow:
End ::= [APPLICATION 4] SEQUENCE {
  dtid             DestTransactionID,
  dialoguePortion  DialoguePortion OPTIONAL,
  components       ComponentPortion OPTIONAL
} */
class TETEnd : public asn1::ber::EncoderOfStructure_T<3> {
protected:
  TEDestTransactionId           _trIdDst;
  //NOTE: only optional AARE_APdu is allowed
  asn1::ber::EncoderProducer_T<TEDialoguePortionStructured> _partDlg;
  //NOTE: returnable Invocations are forbidden
  asn1::ber::EncoderProducer_T<TEComponentPortion>          _partComp;

  TEDialoguePortionStructured * initDlgPortion(void);
  TEComponentPortion * initCompPortion(void);

public:
  static const asn1::ASTagging _typeTags;

  explicit TETEnd(uint32_t remote_tr_id,
                  asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<3>(_typeTags, use_rule)
    , _trIdDst(remote_tr_id, use_rule)
  {
    asn1::ber::EncoderOfStructure_T<3>::setField(0, _trIdDst);
  }
  ~TETEnd()
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

#endif/* __EYELINE_TCAP_PROTO_TEND_HPP */

