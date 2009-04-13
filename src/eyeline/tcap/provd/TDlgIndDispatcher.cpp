#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/ASNTags.hpp"
#include "eyeline/tcap/provd/TDlgIndDispatcher.hpp"
#include "eyeline/tcap/provd/SUAIndDispatcher.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::asn1::ASTag;
using eyeline::asn1::BITBuffer;

using eyeline::tcap::proto::TCMsgAbortPortion;

/* ************************************************************************* *
 * class TDlgIndicationDispatcher implementation
 * ************************************************************************* */
void TDlgIndicationDispatcher::Reset(IKind_e use_ikind/* = indNone*/)
{
  resetObj();
  switch (_kind = use_ikind) {
  case indTBegin:   _dsp.tBegin = new(objMem)TBeginIndDispatcher(_msgTC); break;
  case indTCont:    _dsp.tCont = new(objMem)TContIndDispatcher(_msgTC); break;
  case indTEnd:     _dsp.tEnd = new(objMem)TEndIndDispatcher(_msgTC); break;
  case indTPAbrt:   _dsp.tPAbrt = new(objMem)TPAbortIndDispatcher(_msgTC); break;
  case indTUAbrt:   _dsp.tUAbrt = new(objMem)TUAbortIndDispatcher(_msgTC); break;
  case indTNotice:  _dsp.tNotice = new(objMem)TNoticeIndDispatcher(_msgTC); break;
  default:; //indNone:
  }
  return;
}

bool TDlgIndicationDispatcher::processSuaMsgBuf(void)
{
  SUAIndicationParser  suaParser;
  return suaParser.parseMsgInfo(this->suaMsgBuf()) ?
                  suaParser.processInd(*this) : false;
}

// ----------------------------------------
// -- SUAIndHandlerIface interface methods
// ----------------------------------------
bool TDlgIndicationDispatcher::processSuaInd(const SUAUnitdataInd & sua_ind)
{
  ASTag tmTag(sua_ind.userData(), sua_ind.userDataLen());
  if (tmTag.tagClass != ASTag::tagApplication)
    return false;

  uint32_t pos = sua_ind.userData() - sua_ind.msgBuffer();
  BITBuffer tmsgEnc(sua_ind.msgBuffer() + pos, (uint32_t)sua_ind.userDataLen());

  //decode TCAP message and create associated dispatcher
  switch (tmTag.tagValue) {
  case TCAPMessage::t_begin: {
    _msgTC.Reset(TCAPMessage::t_begin);
    _msgTC.DeferredDecode(tmsgEnc);
    Reset(indTBegin);
    _dsp.tBegin->bindSUAInd(sua_ind);
  } break;

  case TCAPMessage::t_end: {
    _msgTC.Reset(TCAPMessage::t_end);
    _msgTC.DeferredDecode(tmsgEnc);
    Reset(indTEnd);
    _dsp.tEnd->bindSUAInd(sua_ind);
  } break;

  case TCAPMessage::t_continue: {
    _msgTC.Reset(TCAPMessage::t_continue);
    _msgTC.DeferredDecode(tmsgEnc);
    Reset(indTCont);
    _dsp.tCont->bindSUAInd(sua_ind);
  } break;

  case TCAPMessage::t_abort: {
    _msgTC.Reset(TCAPMessage::t_abort);
    _msgTC.DeferredDecode(tmsgEnc);
    if (_msgTC.AbortPortion()->AbortForm() == TCMsgAbortPortion::abrtFrmProvider) {
      Reset(indTPAbrt);
      _dsp.tPAbrt->bindSUAInd(sua_ind);
    } else{
      Reset(indTUAbrt);
      _dsp.tUAbrt->bindSUAInd(sua_ind);
    }
  } break;

  default: //t_unidirection, t_none
    return false;
  }
  return true;
}

bool TDlgIndicationDispatcher::processSuaInd(const SUANoticeInd & sua_ind)
{
  ASTag tmTag(sua_ind.userData(), sua_ind.userDataLen());
  if (tmTag.tagClass != ASTag::tagApplication)
    return false;

  uint32_t pos = sua_ind.userData() - sua_ind.msgBuffer();
  BITBuffer tmsgEnc(sua_ind.msgBuffer() + pos, (uint32_t)sua_ind.userDataLen());

  //decode TCAP message
  switch (tmTag.tagValue) {
  case TCAPMessage::t_begin:
  case TCAPMessage::t_end:
  case TCAPMessage::t_continue:
  case TCAPMessage::t_abort: {
    _msgTC.Reset(static_cast<TCAPMessage::TKind_e>(tmTag.tagValue));
    _msgTC.DeferredDecode(tmsgEnc);
  } break;

  default: //t_unidirection, t_none
    return false;
  }
  //create TNoticeInd dispatcher
  Reset(indTNotice);
  _dsp.tNotice->bindSUAInd(sua_ind);
  return true;
}


} //provd
} //tcap
} //eyeline

