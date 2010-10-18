/* ************************************************************************* *
 * TCAP Begin Message Encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_TBEGIN_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_TBEGIN_HPP

#include "eyeline/tcap/proto/enc/TETransactionId.hpp"
#include "eyeline/tcap/proto/enc/TEDialoguePortion.hpp"
#include "eyeline/tcap/proto/enc/TEComponentPortion.hpp"

#include "eyeline/asn1/BER/rtenc/EncoderProducer.hpp"

namespace eyeline {
namespace tcap {
namespace proto {
namespace enc {

/* Begin is defined in IMPLICIT tagging environment as follow:
Begin ::= [APPLICATION 2] SEQUENCE {
  otid             OrigTransactionID,
  dialoguePortion  DialoguePortion OPTIONAL,
  components       ComponentPortion OPTIONAL
} */
class TETBegin : public asn1::ber::EncoderOfStructure_T<3> {
protected:
  TEOrigTransactionId           _trIdSrc;
  asn1::ber::EncoderProducer_T<TEDialoguePortionStructured> _partDlg;   //NOTE: only optional AARQ_APdu is allowed
  asn1::ber::EncoderProducer_T<TEComponentPortion>          _partComp;  //NOTE: only Invocations are allowed

  TEDialoguePortionStructured * initDlgPortion(void);
  TEComponentPortion * initCompPortion(void);

public:
  static const asn1::ASTagging _typeTags;

  explicit TETBegin(uint32_t local_tr_id,
                    asn1::TransferSyntax::Rule_e use_rule = asn1::TransferSyntax::ruleDER)
    : asn1::ber::EncoderOfStructure_T<3>(_typeTags, use_rule)
    , _trIdSrc(local_tr_id, use_rule)
  {
    asn1::ber::EncoderOfStructure_T<3>::setField(0, _trIdSrc);
  }
  ~TETBegin()
  { }

  //Creates and initializes AARQ_APdu (sets ApplicationContext)
  TEAPduAARQ * initDlgRequest(const asn1::EncodedOID & use_app_ctx)
  {
    return initDlgPortion()->initPduAARQ(use_app_ctx);
  }

  //Creates and appends new component (ROS Pdu) encoder
  void addComponent(const ros::ROSPdu & ros_pdu) /*throw(std::exception)*/
  {
    initCompPortion()->addElementValue(ros_pdu);
  }

  void addComponents(const tcap::TComponentsPtrList & comp_list) /*throw(std::exception)*/
  {
    initCompPortion()->setValue(comp_list);
  }

  void clearDlgPortion(void);

  void clearCompPortion(void);
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_TBEGIN_HPP */

