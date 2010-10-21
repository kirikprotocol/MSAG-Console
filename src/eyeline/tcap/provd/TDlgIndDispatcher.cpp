#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

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
    return _dsp.tBegin().init().processSCSPInd(scsp_ind, *_msgTC.begin().get());
  }
  case proto::TCMessage::t_end: {
    return _dsp.tEnd().init().processSCSPInd(scsp_ind, *_msgTC.end().get());
  }
  case proto::TCMessage::t_continue: {
    return _dsp.tCont().init().processSCSPInd(scsp_ind, *_msgTC.cont().get());
  }
  case proto::TCMessage::t_abort: {
    if (_msgTC.abort().get()->isByProvider()) {
      return _dsp.tPAbrt().init().processSCSPInd(scsp_ind, *_msgTC.abort().get());
    }
    //NOTE: user requested abort may have no reason specified!!!
    return _dsp.tUAbrt().init().processSCSPInd(scsp_ind, *_msgTC.abort().get());
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
  return _dsp.tNotice().init().processSCSPInd(scsp_ind);
}


} //provd
} //tcap
} //eyeline

