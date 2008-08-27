#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <sys/types.h>
#include <vector>

#include "util/ObjectRegistry.hpp"

#include "inman/inap/dispatcher.hpp"
#include "inman/common/cvtutil.hpp"
#include "inman/comp/map_uss/MapUSSFactory.hpp"
using smsc::inman::comp::_ac_map_networkUnstructuredSs_v2;

#include "USSRequestProcessor.hpp"

namespace smsc {
namespace inman {
namespace uss {

USSRequestProcessor::USSRequestProcessor(smsc::inman::interaction::Connect* conn,
                                         const UssService_CFG& cfg,
                                         const USSProcSearchCrit& ussProcSearchCrit,
                                         Logger * use_log/* = NULL*/)
  : _conn(conn), _logger(use_log ? use_log : Logger::getInstance("smsc.ussman.BalanceService")),
    _mapDialog(NULL), _cfg(cfg), _dialogId(0), _resultAsLatin1(false), _dcs(0),
    _ussProcSearchCrit(ussProcSearchCrit)
{ }

USSRequestProcessor::~USSRequestProcessor()
{
  try {
    smsc::core::synchronization::MutexGuard mg(_callbackActivityLock);
    if ( _mapDialog) {
        _mapDialog->endMapDlg();
        delete _mapDialog;
    }
  } catch(...) {}
}

void
USSRequestProcessor::setDialogId(uint32_t dialogId)
{
  _dialogId = dialogId;
}

void
USSRequestProcessor::sendNegativeResponse()
{
  smsc::inman::interaction::SPckUSSResult resultPacket;
  resultPacket.Cmd().setStatus(smsc::inman::interaction::USS2CMD::STATUS_USS_REQUEST_FAILED);

  smsc_log_debug(_logger, "USSRequestProcessor::sendNegativeResponse::: send negative response=[%s]",
                 resultPacket.Cmd().toString().c_str());
  resultPacket.setDialogId(_dialogId);
  if ( _conn && _conn->sendPck(&resultPacket) == -1 ) {
    _conn->Close(); _conn=NULL;
  }
}

void
USSRequestProcessor::handleRequest(const smsc::inman::interaction::USSRequestMessage* requestObject)
{
  TCSessionSR * _mapSess = getMAPSession(requestObject->get_IN_SSN(),
                    requestObject->get_IN_ISDNaddr(), "USSRequestProcessor::handleRequest");
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
  } catch (std::exception & ex) {
    smsc_log_error(_logger, "USSRequestProcessor::handleRequest::: catch exception [%s]", ex.what());
    sendNegativeResponse();
    throw;
  }
}

void USSRequestProcessor::onMapResult(smsc::inman::comp::uss::MAPUSS2CompAC* arg)
{
  smsc::core::synchronization::MutexGuard mg(_callbackActivityLock);
  if ( arg->getUSSDataAsLatin1Text(_resultUssAsString) ) {
    _resultAsLatin1 = true;
    smsc_log_debug(_logger, "USSRequestProcessor::onMapResult::: got USSData=[%s]",
                   _resultUssAsString.c_str());
  } else {
    _resultUssData = arg->getUSSData();
    _dcs = arg->getDCS();

    smsc_log_debug(_logger, "USSRequestProcessor::onMapResult::: got USSData=[%s], _dcs=%d",
                   smsc::util::DumpHex(_resultUssData.size(), &_resultUssData[0]).c_str(), _dcs);
  }
}

//dialog finalization/error handling:
//if ercode != 0, no result has been got from MAP service
void USSRequestProcessor::onEndMapDlg(RCHash ercode/* =0*/)
{
  {
    smsc::core::synchronization::MutexGuard mg(_callbackActivityLock);
    smsc::inman::interaction::SPckUSSResult resultPacket;
    if (!ercode) {
      // success and send result 
      // process std::vector result
      smsc_log_debug(_logger, "USSRequestProcessor::onEndMapDlg::: _dcs=%02X _resultAsLatin1=%d", _dcs, _resultAsLatin1);
      if ( _resultAsLatin1 )
        resultPacket.Cmd().setUSSData(_resultUssAsString.c_str(), (unsigned)_resultUssAsString.size());
      else {
        smsc::cbs::CBS_DCS cbs;
        if (smsc::cbs::parseCBS_DCS(_dcs, cbs) == smsc::cbs::CBS_DCS::dcUCS2 && 
            !cbs.compressed &&
            !cbs.UDHind ) {
          smsc_log_debug(_logger, "USSRequestProcessor::onEndMapDlg::: got UCS2 encoded message response");
          if ( cbs.lngPrefix == smsc::cbs::CBS_DCS::lng4UCS2 )
            resultPacket.Cmd().setUCS2USSData(std::vector<uint8_t>(&_resultUssData[0]+2,
                                                                   &_resultUssData[0]+_resultUssData.size()));
          else
            resultPacket.Cmd().setUCS2USSData(_resultUssData);
        } else
          resultPacket.Cmd().setRAWUSSData(_dcs, _resultUssData);
      }
      resultPacket.Cmd().setStatus(smsc::inman::interaction::USS2CMD::STATUS_USS_REQUEST_OK);
      resultPacket.Cmd().setMSISDNadr(_msISDNAddr);
    } else {
      smsc_log_error(_logger, "USSRequestProcessor::onEndMapDlg: error %u: %s",
                     ercode, URCRegistry::explainHash(ercode).c_str());
      resultPacket.Cmd().setStatus(smsc::inman::interaction::USS2CMD::STATUS_USS_REQUEST_FAILED);
    }

    smsc_log_debug(_logger, "USSRequestProcessor::onEndMapDlg::: send response object=[%s]",
                   resultPacket.Cmd().toString().c_str());
    resultPacket.setDialogId(_dialogId);
    if ( _conn && _conn->sendPck(&resultPacket) == -1 ) {
      _conn->Close(); _conn=NULL;
    }

    delete _mapDialog; _mapDialog=NULL;
    _resultAsLatin1 = false; _dcs = 0; _resultUssData.clear();

    //  smsc::util::RefObjectRegistry<USSRequestProcessor,USSProcSearchCrit>::getInstance().toUnregisterObject(_ussProcSearchCrit);
    DuplicateRequestChecker::getInstance().unregisterRequest(_ussProcSearchCrit);
  }
  delete this;
}


TCSessionSR * USSRequestProcessor::getMAPSession(uint8_t rmt_ssn, const TonNpiAddress & rnpi,
                                        const char * _logId)
{
    if (_cfg.tcDisp->ss7State() != smsc::inman::inap::TCAPDispatcherITF::ss7CONNECTED) {
        smsc_log_error(_logger, "%s: TCAPDispatcher is not connected!", _logId);
        return NULL;
    }
    SSNSession * ssnSess = _cfg.tcDisp->openSSN(_cfg.tcUsr.ownSsn, _cfg.tcUsr.maxDlgId, 1, _logger);
    if (!ssnSess || (ssnSess->getState() != smsc::inman::inap::ssnBound)) {
        smsc_log_error(_logger, "%s: SSN[%u] is not available/bound!", _logId,
                       (unsigned)_cfg.tcUsr.ownSsn);
        return NULL;
    }
    TCSessionSR * mapSess = ssnSess->newSRsession(_cfg.tcUsr.ownAddr,
                _ac_map_networkUnstructuredSs_v2, rmt_ssn, rnpi, _cfg.tcUsr.fakeSsn);
    if (!mapSess) {
        std::string sid = ssnSess->mkSignature(_cfg.tcUsr.ownAddr,
                                     _ac_map_networkUnstructuredSs_v2, rmt_ssn, &rnpi);
        smsc_log_error(_logger, "%s: Unable to init TCSR session: %s", _logId, sid.c_str());
    } else
        smsc_log_debug(_logger, "%s: using TCSR[%u]: %s", _logId, mapSess->getUID(),
                       mapSess->Signature().c_str());
    return mapSess;
}

}
}
}
