/* ************************************************************************* *
 * TCAP End Message Encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_TEND_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_TEND_HPP

#include "eyeline/tcap//TComponentDefs.hpp"
#include "eyeline/tcap/proto/enc/TETransactionId.hpp"
#include "eyeline/tcap/proto/enc/TEDialoguePortion.hpp"
#include "eyeline/tcap/proto/enc/TEComponentPortion.hpp"

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
private:
  using asn1::ber::EncoderOfStructure_T<3>::addField;
  using asn1::ber::EncoderOfStructure_T<3>::setField;

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
  TEDestTransactionId           _trIdDst;
  TEDialoguePortionStructured * _partDlg; //NOTE: only optional AARE_APdu is allowed
  TEComponentPortion *          _partComp;

  TEDialoguePortionStructured * getDlgPortion(void)
  {
    if (!_partDlg) {
      _partDlg = new (_memDlg.buf)TEDialoguePortionStructured(TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfStructure_T<3>::setField(1, *_partDlg);
    }
    return _partDlg;
  }

  TEComponentPortion * getCompPortion(void)
  {
    if (!_partComp) {
      _partComp = new (_memComp.buf)TEComponentPortion(TSGroupBER::getBERRule(getTSRule()));
      asn1::ber::EncoderOfStructure_T<3>::setField(2, *_partComp);
    }
    return _partComp;
  }

  //Creates and appends new component to components list
  TEComponent * initComponent(void)
  {
    return getCompPortion()->addComponent();
  }

public:
  static const asn1::ASTagging _typeTags;

  explicit TETEnd(uint32_t remote_tr_id, TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfStructure_T<3>(_typeTags, TSGroupBER::getTSRule(use_rule))
    , _trIdDst(remote_tr_id, use_rule), _partDlg(NULL), _partComp(NULL)
  {
    _memDlg.aligner = _memComp.aligner = 0;
    asn1::ber::EncoderOfStructure_T<3>::addField(_trIdDst);
  }
  ~TETEnd()
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

  //Creates and appends new component (ROS Pdu) to components list
  void addComponent(const ros::ROSPduPrimitiveAC & ros_req)
  {
    switch (ros_req.getKind()) {
    case ros::ROSPduPrimitiveAC::rosInvoke:
      initComponent()->setInvoke(static_cast<const ros::ROSInvokePdu &>(ros_req)); break;
    case ros::ROSPduPrimitiveAC::rosResult:
      initComponent()->setResult(static_cast<const ros::ROSResultPdu &>(ros_req)); break;
    case ros::ROSPduPrimitiveAC::rosError:
      initComponent()->setError(static_cast<const ros::ROSErrorPdu &>(ros_req)); break;
    case ros::ROSPduPrimitiveAC::rosReject:
      initComponent()->setReject(static_cast<const ros::ROSRejectPdu &>(ros_req)); break;
    case ros::ROSPduPrimitiveAC::rosResultNL:
      initComponent()->setResultNL(static_cast<const ros::ROSResultNLPdu &>(ros_req)); break;
    //case ROSPduPrimitiveAC::rosCancel:
    default:;
    }
  }

  void addComponents(const tcap::TComponentsPtrList & comp_list)
  {
    if (!comp_list.empty()) {
      for (tcap::TComponentsPtrList::const_iterator cit = comp_list.begin(); cit != comp_list.end(); ++cit)
        addComponent(**cit);
    }
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

#endif/* __EYELINE_TCAP_PROTO_TEND_HPP */

