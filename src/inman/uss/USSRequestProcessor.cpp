#include "USSRequestProcessor.hpp"
#include <inman/inap/dispatcher.hpp>

namespace smsc {
namespace inman {
namespace uss {

USSRequestProcessor::USSRequestProcessor(smsc::inman::interaction::Connect* conn,
                                         const UssService_CFG& cfg)
  : _conn(conn), _logger(Logger::getInstance("smsc.uss.BalanceService")),
    _mapSess(NULL), _mapDialog(NULL), _cfg(cfg), _dialogId(0) {}

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
  _conn->sendPck(&resultPacket);
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

    _mapDialog->requestSS(requestObject->getUSSData(),  requestObject->getDCS(),
                          &(requestObject->getMSISDNadr()));

    _msISDNAddr = requestObject->getMSISDNadr();
  } catch (std::exception & ex) {
    smsc_log_error(_logger, "USSRequestProcessor::handleRequest::: catch exception [%s]", ex.what());
    sendNegativeResponse();
    throw;
  }
}

void USSRequestProcessor::onMapResult(smsc::inman::comp::uss::MAPUSS2CompAC* arg)
{
  _resultUssData = arg->getUSSData();
  _dcs = arg->getDCS();

  std::string ussAsString;
  if ( arg->getUSSDataAsLatin1Text(ussAsString) )
    smsc_log_debug(_logger, "USSRequestProcessor::onMapResult::: got USSData=[%s]",
                   ussAsString.c_str());
  else
    smsc_log_debug(_logger, "USSRequestProcessor::onMapResult::: got USSData=[%s]",
                   smsc::util::DumpHex(_resultUssData.size(), &_resultUssData[0]).c_str());
}

//dialog finalization/error handling:
//if ercode != 0, no result has been got from MAP service
void USSRequestProcessor::onEndMapDlg(RCHash ercode/* =0*/)
{
  smsc::inman::interaction::SPckUSSResult resultPacket;
  if (!ercode) {
    // success and send result 
    // process std::vector result
    resultPacket.Cmd().setStatus(smsc::inman::interaction::USS2CMD::STATUS_USS_REQUEST_OK);
    // resultPacket.Cmd().setUSSData(_resultUssData);
    resultPacket.Cmd().setRAWUSSData(_dcs, _resultUssData);
    resultPacket.Cmd().setMSISDNadr(_msISDNAddr);
  } else {
      smsc_log_error(_logger, "USSRequestProcessor::onEndMapDlg: error %u: %s", _logger,
          ercode, URCRegistry::explainHash(ercode).c_str());
      resultPacket.Cmd().setStatus(smsc::inman::interaction::USS2CMD::STATUS_USS_REQUEST_FAILED);
  }

  smsc_log_debug(_logger, "USSRequestProcessor::onEndMapDlg::: send response object=[%s]",
                 resultPacket.Cmd().toString().c_str());
  resultPacket.setDialogId(_dialogId);
  _conn->sendPck(&resultPacket);

  delete _mapDialog;
}

}
}
}
