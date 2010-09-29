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
class TETContinue : public asn1::ber::EncoderOfPlainStructure_T<4> {
private:
  using asn1::ber::EncoderOfPlainStructure_T<4>::addField;
  using asn1::ber::EncoderOfPlainStructure_T<4>::setField;

  union {
    void * aligner;
    uint64_t buf[eyeline::util::MultiplierOfSize_T<
                    sizeof(TEDialoguePortionStructured), uint64_t>::VALUE];
  } _memDlg;
  union {
    void * aligner;
    uint64_t buf[eyeline::util::MultiplierOfSize_T<
                    sizeof(TEComponentPortion), uint64_t>::VALUE];
  } _memComp;

protected:
  TEOrigTransactionId           _trIdSrc;
  TEDestTransactionId           _trIdDst;
  TEDialoguePortionStructured * _partDlg; //NOTE: only optional AARE_APdu is allowed
  TEComponentPortion *          _partComp;

  TEDialoguePortionStructured * getDlgPortion(void)
  {
    if (!_partDlg) {
      _partDlg = new (_memDlg.buf)TEDialoguePortionStructured(TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfPlainStructure_T<4>::setField(2, *_partDlg);
    }
    return _partDlg;
  }

  TEComponentPortion * getCompPortion(void)
  {
    if (!_partComp) {
      _partComp = new (_memComp.buf)TEComponentPortion(TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfPlainStructure_T<4>::setField(3, *_partComp);
    }
    return _partComp;
  }

public:
  static const asn1::ASTagging _typeTags;

  explicit TETContinue(uint32_t local_tr_id, uint32_t remote_tr_id,
                     TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfPlainStructure_T<4>(_typeTags, TSGroupBER::getTSRule(use_rule))
    , _trIdSrc(local_tr_id, use_rule), _trIdDst(remote_tr_id, use_rule)
    , _partDlg(NULL), _partComp(NULL)
  {
    _memDlg.aligner = _memComp.aligner = 0;
    asn1::ber::EncoderOfPlainStructure_T<4>::addField(_trIdSrc);
    asn1::ber::EncoderOfPlainStructure_T<4>::addField(_trIdDst);
  }
  ~TETContinue()
  {
    if (_partDlg)
      _partDlg->~TEDialoguePortionStructured();
    clearCompPortion();
  }

  // --------------------------------------------------------------------
  //NOTE: calling DialoguePortion initialization methods is allowed only
  //      in case of TEnd is an immediate response to TBegin
  // --------------------------------------------------------------------
  //Creates and initializes AARE_APdu (sets ApplicationContext)
  TEAPduAARE * initDlgResponse(const asn1::EncodedOID & use_app_ctx)
  {
    return getDlgPortion()->getPduAARE(use_app_ctx);
  }

  //Creates and appends new component (ROS Pdu) encoder
  void addComponent(const ros::ROSPdu & ros_pdu)
  {
    getCompPortion()->addValue(ros_pdu);
  }

  void addComponents(const tcap::TComponentsPtrList & comp_list)
  {
    getCompPortion()->addValuesList(comp_list);
  }

  void clearCompPortion(void)
  {
    if (_partComp) {
      _partComp->~TEComponentPortion();
      _partComp = 0;
    }
  }
};

}}}}

#endif/* __EYELINE_TCAP_PROTO_TCONT_HPP */

