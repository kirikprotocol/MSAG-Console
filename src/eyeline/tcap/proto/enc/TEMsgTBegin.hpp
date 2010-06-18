/* ************************************************************************* *
 * TCAP Begin Message Encoder.
 * ************************************************************************* */
#ifndef __EYELINE_TCAP_PROTO_TBEGIN_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROTO_TBEGIN_HPP

#include "eyeline/tcap/proto/enc/TETransactionId.hpp"
#include "eyeline/tcap/proto/enc/TEDialoguePortion.hpp"
#include "eyeline/tcap/proto/enc/TEComponentPortion.hpp"

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
  TEOrigTransactionId           _trIdSrc;
  TEDialoguePortionStructured * _partDlg; //NOTE: only optional AARQ_APdu is allowed
  TEComponentPortion *          _partComp; //NOTE: only Invokations are allowed

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

public:
  static const asn1::ASTagging _typeTags;

  explicit TETBegin(uint32_t local_tr_id, TSGroupBER::Rule_e use_rule = TSGroupBER::ruleDER)
    : asn1::ber::EncoderOfStructure_T<3>(_typeTags, TSGroupBER::getTSRule(use_rule))
    , _trIdSrc(local_tr_id, use_rule), _partDlg(NULL), _partComp(NULL)
  {
    _memDlg.aligner = _memComp.aligner = 0;
    asn1::ber::EncoderOfStructure_T<3>::addField(_trIdSrc);
  }
  ~TETBegin()
  {
    if (_partDlg)
      _partDlg->~TEDialoguePortionStructured();
    clearCompPortion();
  }

  //Creates and initializes AARQ_APdu (sets ApplicationContext)
  TEAPduAARQ * initDlgRequest(const asn1::EncodedOID & use_app_ctx)
  {
    return getDlgPortion()->getPduAARQ(use_app_ctx);
  }

  //Creates and appends new Invpoke to components list
  void addInvoke(const ros::ROSInvokePdu & req_inv)
  {
    getCompPortion()->addComponent()->setInvoke(req_inv);
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

#endif/* __EYELINE_TCAP_PROTO_TBEGIN_HPP */

