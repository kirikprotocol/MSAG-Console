#include "USSRequestProcessor.hpp"
#include <inman/inap/dispatcher.hpp>
#include <inman/common/cvtutil.hpp>
#include <vector>
#include <sys/types.h>

namespace smsc {
namespace inman {
namespace uss {

USSRequestProcessor::USSRequestProcessor(smsc::inman::interaction::Connect* conn,
                                         const UssService_CFG& cfg)
  : _conn(conn), _logger(Logger::getInstance("smsc.uss.BalanceService")),
    _mapSess(NULL), _mapDialog(NULL), _cfg(cfg), _dialogId(0), _resultAsLatin1(false), _dcs(0) {}

USSRequestProcessor::~USSRequestProcessor() {
  try {
    smsc::core::synchronization::MutexGuard mg(_callbackActivityLock);
    if ( _mapDialog) _mapDialog->endMapDlg();
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
  smsc::inman::inap::TCAPDispatcher * disp = smsc::inman::inap::TCAPDispatcher::getInstance();
  smsc::inman::inap::SSNSession * ssnSess = disp->findSession(_cfg.ss7.own_ssn);
  if (!ssnSess) //attempt to open SSN
    ssnSess = disp->openSSN(_cfg.ss7.own_ssn, _cfg.ss7.maxDlgId);

  if (!ssnSess || (ssnSess->getState() != smsc::inman::inap::ssnBound)) {
    smsc_log_error(_logger, "USSRequestProcessor::handleRequest::: SSN session is not available/bound");
    sendNegativeResponse();
    return;
  }

  smsc::util::TonNpiAddress IN_ISDNAddr = requestObject->get_IN_ISDNaddr();
  if ( !_mapSess ) 
    if (!(_mapSess = ssnSess->newSRsession(_cfg.ss7.ssf_addr,
                                           ACOID::id_ac_map_networkUnstructuredSs_v2,
                                           requestObject->get_IN_SSN(),
                                           IN_ISDNAddr,
                                           _cfg.ss7.fake_ssn))) {
      std::string sid;
      smsc::inman::inap::TCSessionAC::mkSignature(sid, _cfg.ss7.own_ssn, _cfg.ss7.ssf_addr,
                                                  ACOID::id_ac_cap3_sms_AC, requestObject->get_IN_SSN(),
                                                  &IN_ISDNAddr);
      smsc_log_error(_logger, "USSRequestProcessor::handleRequest::: Unable to init TCSR session: %s", sid.c_str());
      sendNegativeResponse();
      return;
    }

  try {
    _mapDialog = new smsc::inman::inap::uss::MapUSSDlg(_mapSess, this);

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
  smsc::core::synchronization::MutexGuard mg(_callbackActivityLock);
  smsc::inman::interaction::SPckUSSResult resultPacket;
  if (!ercode) {
    // success and send result 
    // process std::vector result
    smsc_log_debug(_logger, "USSRequestProcessor::onEndMapDlg::: _dcs=%02X _resultAsLatin1=%d", _dcs, _resultAsLatin1);
    if ( _resultAsLatin1 )
      resultPacket.Cmd().setUSSData(_resultUssAsString.c_str(), _resultUssAsString.size());
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
}

}
}
}
