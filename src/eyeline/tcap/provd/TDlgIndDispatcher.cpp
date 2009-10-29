#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/asn1/BER/rtdec/TLVDecoder.hpp"
#include "eyeline/tcap/provd/TDlgIndDispatcher.hpp"
#include "eyeline/tcap/provd/SUAIndDispatcher.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::asn1::ASTag;
using eyeline::asn1::BITBuffer;
using eyeline::asn1::DECResult;
using eyeline::asn1::ber::decode_tag;

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
  return suaParser.parseMsgInfo(*(this->suaMsgBuf())) ?
                  suaParser.processInd(*this) : false;
}

// ----------------------------------------
// -- SUAIndHandlerIface interface methods
// ----------------------------------------
bool TDlgIndicationDispatcher::processSuaInd(const SUAUnitdataInd & sua_ind)
{
  ASTag tmTag;
  DECResult decRc = decode_tag(tmTag, sua_ind.userData(), sua_ind.userDataLen());
  if (decRc.status || (tmTag._tagClass != ASTag::tagApplication))
    return false;

  bool      failed = false;
  uint32_t  pos = (uint32_t)(sua_ind.userData() - sua_ind.msgBuffer());
  BITBuffer tmsgEnc(sua_ind.msgBuffer() + pos, (uint32_t)sua_ind.userDataLen());

  //decode TCAP message and create associated dispatcher
  switch (tmTag._tagValue) {
  case TCAPMessage::t_begin: {
    _msgTC.Reset(TCAPMessage::t_begin);
    decRc = _msgTC.deferredDecode(tmsgEnc);
    if (decRc.status) {
      failed = true;
    } else {
      Reset(indTBegin);
      failed |= _dsp.tBegin->bindSUAInd(sua_ind);
    }
  } break;

  case TCAPMessage::t_end: {
    _msgTC.Reset(TCAPMessage::t_end);
    decRc = _msgTC.deferredDecode(tmsgEnc);
    if (decRc.status) {
      failed = true;
    } else {
      Reset(indTEnd);
      failed |= _dsp.tEnd->bindSUAInd(sua_ind);
    }
  } break;

  case TCAPMessage::t_continue: {
    _msgTC.Reset(TCAPMessage::t_continue);
    decRc = _msgTC.deferredDecode(tmsgEnc);
    if (decRc.status) {
      failed = true;
    } else {
      Reset(indTCont);
      failed |= _dsp.tCont->bindSUAInd(sua_ind);
    }
  } break;

  case TCAPMessage::t_abort: {
    _msgTC.Reset(TCAPMessage::t_abort);
    decRc = _msgTC.deferredDecode(tmsgEnc);
    if (decRc.status) {
      failed = true;
    } else {
      if (_msgTC.getAbortPortion()->getAbortForm() == TCMsgAbortPortion::abrtFrmProvider) {
        Reset(indTPAbrt);
        failed |= _dsp.tPAbrt->bindSUAInd(sua_ind);
      } else{
        Reset(indTUAbrt);
        failed |= _dsp.tUAbrt->bindSUAInd(sua_ind);
      }
    }
  } break;

  default: //t_unidirection, t_none
    failed = true;
  }
  return !failed;
}

bool TDlgIndicationDispatcher::processSuaInd(const SUANoticeInd & sua_ind)
{
  ASTag tmTag;
  DECResult decRc = decode_tag(tmTag, sua_ind.userData(), sua_ind.userDataLen());
  if (decRc.status || (tmTag._tagClass != ASTag::tagApplication))
    return false;

  bool      failed = false;
  uint32_t  pos = (uint32_t)(sua_ind.userData() - sua_ind.msgBuffer());
  BITBuffer tmsgEnc(sua_ind.msgBuffer() + pos, (uint32_t)sua_ind.userDataLen());

  //decode TCAP message
  switch (tmTag._tagValue) {
  case TCAPMessage::t_begin:
  case TCAPMessage::t_end:
  case TCAPMessage::t_continue:
  case TCAPMessage::t_abort: {
    _msgTC.Reset(static_cast<TCAPMessage::TKind_e>(tmTag._tagValue));
    decRc = _msgTC.deferredDecode(tmsgEnc);
    if (decRc.status)
      failed = true;
  } break;

  default: //t_unidirection, t_none
    failed = true;
  }
  //create TNoticeInd dispatcher
  if (!failed) {
    Reset(indTNotice);
    failed = _dsp.tNotice->bindSUAInd(sua_ind);
  }
  return !failed;
}


} //provd
} //tcap
} //eyeline

