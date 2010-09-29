#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/TDlgIndDispatcher.hpp"
#include "eyeline/tcap/provd/SCSPIndDispatcher.hpp"
#include "eyeline/tcap/proto/dec/TDTCMessage.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

/* ************************************************************************* *
 * class TDlgIndicationDispatcher implementation
 * ************************************************************************* */
bool TDlgIndicationDispatcher::processSCSPMessage(void) /*throw(std::exception)*/
{
  SCSPIndicationParser  scspParser;
  return scspParser.parseMsgInfo(_msgSCSP) ? scspParser.processInd(*this) : false;
}

// ----------------------------------------
// -- SCSPIndHandlerIface interface methods
// ----------------------------------------
bool TDlgIndicationDispatcher::processSCSPInd(const SCSPUnitdataInd & scsp_ind)
  /*throw(std::exception)*/
{
  proto::dec::TDTCMessage _msgDec(_msgTC);
  asn1::DECResult decRc = _msgDec.decode(scsp_ind.userData(), scsp_ind.userDataLen()); //throws!!!
  if (!decRc.isOk() || !_msgTC.verifyPdu())
    return false; //TODO: log something meaningfull

  //create associated indication dispatcher
  switch (_msgTC.getKind()) {
  case proto::TCMessage::t_begin: {
    _dsp.tBegin = new (_objMem._buf) TBeginIndDispatcher();
    return _dsp.tBegin->processSCSPInd(scsp_ind, *_msgTC.getBegin());
  }
  case proto::TCMessage::t_end: {
    _dsp.tEnd = new (_objMem._buf) TEndIndDispatcher();
    return _dsp.tEnd->processSCSPInd(scsp_ind, *_msgTC.getEnd());
  }
  case proto::TCMessage::t_continue: {
    _dsp.tCont = new (_objMem._buf) TContIndDispatcher();
    return _dsp.tCont->processSCSPInd(scsp_ind, *_msgTC.getContinue());
  }
  case proto::TCMessage::t_abort: {
    if (_msgTC.getAbort()->isByProvider()) {
      _dsp.tPAbrt = new (_objMem._buf) TPAbortIndDispatcher();
      return _dsp.tPAbrt->processSCSPInd(scsp_ind, *_msgTC.getAbort());
    }
    //NOTE: user requested abort may have no reason specified!!!
    _dsp.tUAbrt = new (_objMem._buf) TUAbortIndDispatcher();
    return _dsp.tUAbrt->processSCSPInd(scsp_ind, *_msgTC.getAbort());
  }
  //case proto::TCMessage::t_unidirection:
  //TODO: log something meaningfull
  default:; //t_none
  } //eosw
  return false;
}

bool TDlgIndicationDispatcher::processSCSPInd(const SCSPNoticeInd & scsp_ind)
  /*throw(std::exception)*/
{
  _msgTC.clear();
  _dsp.tNotice = new (_objMem._buf) TNoticeIndDispatcher();
  _dsp.tNotice->processSCSPInd(scsp_ind);
  return true;
}


} //provd
} //tcap
} //eyeline

