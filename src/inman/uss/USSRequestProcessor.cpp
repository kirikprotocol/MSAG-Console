#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <sys/types.h>
#include <vector>

#include "util/ObjectRegistry.hpp"
#include "util/BinDump.hpp"

#include "inman/common/cvtutil.hpp"
#include "inman/inap/dispatcher.hpp"
#include "inman/comp/map_uss/MapUSSFactory.hpp"
using smsc::inman::comp::_ac_map_networkUnstructuredSs_v2;
using smsc::inman::inap::SSNBinding;

#include "inman/uss/USSRequestProcessor.hpp"

namespace smsc {
namespace inman {
namespace uss {

USSRequestProcessor::USSRequestProcessor(USSManConnect* ussManConn, Connect* conn,
                                         const UssService_CFG& cfg, uint32_t dialog_id,
                                         const USSProcSearchCrit& ussProcSearchCrit,
                                         Logger * use_log/* = NULL*/)
  : _isRunning(true), _ussManConn(ussManConn), _conn(conn), _cfg(cfg), _dialogId(dialog_id)
  , _mapDialog(NULL), _resultAsLatin1(false), _dcs(0)
  , _ussProcSearchCrit(ussProcSearchCrit)
  , _logger(use_log ? use_log : Logger::getInstance("smsc.ussman"))
{
  core::synchronization::MutexGuard synchronize(_connLock);
  snprintf(_logId, sizeof(_logId)-1, "USSreq[%u:%u]", _conn->getId(), _dialogId);
}

USSRequestProcessor::~USSRequestProcessor()
{
  {
    core::synchronization::MutexGuard synchronize(_statusLock);
    _isRunning = false;
  }

  try {
    smsc::core::synchronization::MutexGuard mg(_callbackActivityLock);

    if ( _mapDialog) {
      _mapDialog->endMapDlg();
      delete _mapDialog;
    }
  } catch(...) {}

  core::synchronization::MutexGuard synchronize(_connLock);
  if ( _ussManConn ) {
    _ussManConn->markReqProcessorAsCompleted(this);
  }
}

void USSRequestProcessor::sendNegativeResponse()
{
  smsc::inman::interaction::SPckUSSResult resultPacket;
  resultPacket.Cmd().setStatus(smsc::inman::interaction::USS2CMD::STATUS_USS_REQUEST_FAILED);

  smsc_log_warn(_logger, "%s: send negative response=%s", _logId,
                 resultPacket.Cmd().toString().c_str());
  resultPacket.setDialogId(_dialogId);
  sendPacket(&resultPacket);
}

void
USSRequestProcessor::handleRequest(const smsc::inman::interaction::USSRequestMessage* requestObject)
{
  TCSessionSR * _mapSess = getMAPSession(requestObject->get_IN_SSN(),
                                         requestObject->get_IN_ISDNaddr());
  if (!_mapSess) {
    sendNegativeResponse();
    return;
  }
  try {
    _mapDialog = new smsc::inman::inap::uss::MapUSSDlg(_mapSess, this, _logger);

    if ( requestObject->getFlg() == smsc::inman::interaction::USSMessageAC::PREPARED_USS_REQ )
      _mapDialog->requestSS(requestObject->getUSSData(),  requestObject->getDCS(),
                            &(requestObject->getMSISDNadr()), requestObject->getIMSI().c_str());
    else if ( requestObject->getFlg() == smsc::inman::interaction::USSMessageAC::LATIN1_USS_TEXT )
      _mapDialog->requestSS(requestObject->getLatin1Text(),
                            &(requestObject->getMSISDNadr()), requestObject->getIMSI().c_str());

    _msISDNAddr = requestObject->getMSISDNadr();
  } catch (const std::exception & ex) {
    smsc_log_error(_logger, "%s: MapUSSDlg exception %s", _logId, ex.what());
    sendNegativeResponse();
    finalizeRequest();
    delete this;
  }
}

void USSRequestProcessor::onMapResult(smsc::inman::comp::uss::MAPUSS2CompAC* arg)
{
  {
    core::synchronization::MutexGuard synchronize(_statusLock);
    if ( _isRunning == false )
      return;
  }
  core::synchronization::MutexGuard mg(_callbackActivityLock);
  if ( arg->getUSSDataAsLatin1Text(_resultUssAsString) ) {
    _resultAsLatin1 = true;
    smsc_log_debug(_logger, "%s: onMapResult::: got USSData=[%s]", _logId,
                   _resultUssAsString.c_str());
  } else {
    _resultUssData = arg->getUSSData();
    _dcs = arg->getDCS();

    smsc_log_debug(_logger, "%s: onMapResult::: got USSData=[%s], _dcs=%d", _logId,
                   smsc::util::DumpHex(_resultUssData.size(), &_resultUssData[0]).c_str(), _dcs);
  }
}

//dialog finalization/error handling:
//if ercode != 0, no result has been got from MAP service
void USSRequestProcessor::onEndMapDlg(RCHash ercode/* =0*/)
{
  {
    core::synchronization::MutexGuard synchronize(_statusLock);
    if ( _isRunning == false )
      return;
  }

  try {
    core::synchronization::MutexGuard mg(_callbackActivityLock);
    inman::interaction::SPckUSSResult resultPacket;
    if (!ercode) {
      // success and send result
      // process std::vector result
      smsc_log_debug(_logger, "%s: onEndMapDlg::: _dcs=%02X _resultAsLatin1=%d",
                     _logId, _dcs, _resultAsLatin1);
      if ( _resultAsLatin1 )
        resultPacket.Cmd().setUSSData(_resultUssAsString.c_str(), (unsigned)_resultUssAsString.size());
      else {
        cbs::CBS_DCS cbs;
        if (cbs::parseCBS_DCS(_dcs, cbs) == cbs::CBS_DCS::dcUCS2 &&
            !cbs.compressed &&
            !cbs.UDHind ) {
          smsc_log_debug(_logger, "%s: onEndMapDlg::: got UCS2 encoded message response", _logId);
          if ( cbs.lngPrefix == cbs::CBS_DCS::lng4UCS2 )
            resultPacket.Cmd().setUCS2USSData(std::vector<uint8_t>(&_resultUssData[0]+2,
                                                                   &_resultUssData[0]+_resultUssData.size()));
          else
            resultPacket.Cmd().setUCS2USSData(_resultUssData);
        } else
          resultPacket.Cmd().setRAWUSSData(_dcs, _resultUssData);
      }
      resultPacket.Cmd().setStatus(interaction::USS2CMD::STATUS_USS_REQUEST_OK);
      resultPacket.Cmd().setMSISDNadr(_msISDNAddr);
    } else {
      smsc_log_error(_logger, "%s: onEndMapDlg: error %u: %s", _logId,
                     ercode, URCRegistry::explainHash(ercode).c_str());
      resultPacket.Cmd().setStatus(interaction::USS2CMD::STATUS_USS_REQUEST_FAILED);
    }

    smsc_log_info(_logger, "%s: send response=%s", _logId,
                   resultPacket.Cmd().toString().c_str());
    resultPacket.setDialogId(_dialogId);
    sendPacket(&resultPacket);

    finalizeRequest();
  } catch (...) {}
  delete this;
}

TCSessionSR * USSRequestProcessor::getMAPSession(uint8_t rmt_ssn, const TonNpiAddress & rnpi)
{
  if (_cfg.tcDisp->ss7State() != smsc::inman::inap::TCAPDispatcherITF::ss7CONNECTED) {
    smsc_log_error(_logger, "%s: TCAPDispatcher is not connected!", _logId);
    return NULL;
  }
  if (!_cfg.ssnSess || (_cfg.ssnSess->bindStatus() < SSNBinding::ssnPartiallyBound)) {
    smsc_log_error(_logger, "%s: SSN[%u] is not available/bound!", _logId,
                   (unsigned)_cfg.tcUsr.ownSsn);
    return NULL;
  }
  TCSessionSR * mapSess = _cfg.ssnSess->newSRsession(_cfg.tcUsr.ownAddr,
                                                     _ac_map_networkUnstructuredSs_v2,
                                                     rmt_ssn, rnpi, _cfg.tcUsr.fakeSsn);
  if (!mapSess) {
    std::string sid = _cfg.ssnSess->mkSignature(_cfg.tcUsr.ownAddr,
                                                _ac_map_networkUnstructuredSs_v2,
                                                rmt_ssn, &rnpi);
    smsc_log_error(_logger, "%s: Unable to init TCSR session: %s", _logId, sid.c_str());
  } else
    smsc_log_debug(_logger, "%s: using TCSR[%u]: %s", _logId, mapSess->getUID(),
                   mapSess->Signature().c_str());
  return mapSess;
}

void
USSRequestProcessor::sendPacket(inman::interaction::SPckUSSResult* resultPacket)
{
  core::synchronization::MutexGuard synchronize(_connLock);
  try {
    if ( _conn && _conn->sendPck(resultPacket) == -1 ) {
      _conn->Close(); _conn=NULL;
    }
  } catch (std::exception& ex) { _conn=NULL; }
}

void
USSRequestProcessor::markConnectAsClosed()
{
  {
    core::synchronization::MutexGuard synchronize(_statusLock);
    if ( _isRunning == false )
      return;
  }

  core::synchronization::MutexGuard synchronize(_connLock);
  _conn=NULL; _ussManConn=NULL;
}

void
USSRequestProcessor::finalizeRequest()
{
  delete _mapDialog; _mapDialog=NULL;
  _resultAsLatin1 = false; _dcs = 0; _resultUssData.clear();

  DuplicateRequestChecker::getInstance().unregisterRequest(_ussProcSearchCrit);
}

} //uss
} //inman
} //smsc

