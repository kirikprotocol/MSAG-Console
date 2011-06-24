#include "OutputMessageProcessor.hpp"
#include "scag/util/encodings/Encodings.h"
#include "scag/exc/SCAGExceptions.h"

#include "Exceptions.hpp"
#include "advert/BEProtocolV2SimpleClient.hpp"
#include "advert/BEReconnector.hpp"
#include "advert/BannerReader.hpp"

namespace smsc {
namespace mcisme {

OutputMessageProcessor::OutputMessageProcessor(TaskProcessor& task_processor,
                                               util::config::ConfigView* advert_cfg,
                                               OutputMessageProcessorsDispatcher& dispatcher,
                                               BEReconnector& reconnector_thread,
                                               BannerReader& banner_reader)
  : _taskProcessor(task_processor), _isStopped(false), _eventWasSignalled(false),
    _logger(logger::Logger::getInstance("omsgprc")), _handler(NULL),
    _messagesProcessorsDispatcher(dispatcher), _reconnectorThread(reconnector_thread)
{
  if(!advert_cfg->getBool("useAdvert"))
    throw util::Exception("OutputMessageProcessor::OutputMessageProcessor::: Fatal! Advertising.useAdvert = false");

  string advertServer;
  try {
    advertServer = advert_cfg->getString("server");
  } catch (...) {
    advertServer = "0.0.0.0";
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.server> missed. Default value is '0.0.0.0'.");
  }
  int advertPort;
  try {
    advertPort = advert_cfg->getInt("port");
  } catch (...){
    advertPort = 25000;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.port> missed. Default value is '25000'.");
  }

  int connectTimeout;
  try {
    connectTimeout = advert_cfg->getInt("connectTimeout");
    if ( connectTimeout < 0 )
      throw util::Exception("OutputMessageProcessor::OutputMessageProcessor::: invalid Advertising.connectTimeout value < 0");
  } catch (...) {
    connectTimeout = 15;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.connectTimeout> missed. Default value is '15'.");
  }

  int bannerEngineProtocolVersion = advert_cfg->getInt("BEProtocolVersion");
  switch (bannerEngineProtocolVersion) {
  case BEProtocolV2SimpleClient::PROTOCOL_VERSION:
    _advertising = new BEProtocolV2SimpleClient(advertServer, advertPort);
    break;
  default:
    throw util::Exception("OutputMessageProcessor::OutputMessageProcessor::: invalid Advertising.BEProtocolVersion value [%d]", bannerEngineProtocolVersion);
  }

  try {
    _advertising->init(connectTimeout);
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "OutputMessageProcessor::OutputMessageProcessor::: advertising client can't be initialized - caught exception '%s'", ex.what());
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  }
  banner_reader.addClientConnect(_advertising);
}

void
OutputMessageProcessor::stop()
{
  _isStopped = true;
  _outputEventMonitor.notify();
}

int
OutputMessageProcessor::Execute()
{
  while (!_isStopped) {
    try {
      AbntAddr calledAbnt;
      std::auto_ptr<SendMessageEventHandler> handler(waitingForHandler());
      if ( handler.get() ) {
        try {
          handler->handle();
        } catch(...) {
          _messagesProcessorsDispatcher.markMessageProcessorAsFree(this);
          throw;
        }
        _messagesProcessorsDispatcher.markMessageProcessorAsFree(this);
      }
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "OutputMessageProcessor::Execute::: caught unexpected exception '%s'", ex.what());
    } catch (...) {
      smsc_log_error(_logger, "OutputMessageProcessor::Execute::: caught unexpected exception '...'");
    }
  }

  try {
    /*delete _advertising; */_advertising = 0;
  } catch (...) {}

  _messagesProcessorsDispatcher.deleteMessageProcessor(this);

  return 0;
}

void
OutputMessageProcessor::assignMessageOutputWork(const AbntAddr& calledAbnt)
{
  core::synchronization::MutexGuard synchronize(_outputEventMonitor);
  _handler = new SendMissedCallMessageEventHandler(_taskProcessor, _reconnectorThread, _advertising, calledAbnt);

  _eventWasSignalled = true;
  _outputEventMonitor.notify();
}

void
OutputMessageProcessor::assignMessageOutputWork(const sms_info* pInfo, const AbonentProfile& abntProfile)
{
  core::synchronization::MutexGuard synchronize(_outputEventMonitor);
  _handler = new SendAbonentOnlineNotificationEventHandler(_taskProcessor, _reconnectorThread, _advertising, pInfo, abntProfile);

  _eventWasSignalled = true;
  _outputEventMonitor.notify();
}

void
OutputMessageProcessor::assignMessageOutputWork(const BannerResponseTrace& bannerRespTrace)
{
  core::synchronization::MutexGuard synchronize(_outputEventMonitor);
  _handler = new RollbackBERequestHandler(_taskProcessor, _reconnectorThread, _advertising, bannerRespTrace);

  _eventWasSignalled = true;
  _outputEventMonitor.notify();
}

SendMessageEventHandler*
OutputMessageProcessor::waitingForHandler()
{
  core::synchronization::MutexGuard synchronize(_outputEventMonitor);

  while ( !_eventWasSignalled ) {
    if ( _outputEventMonitor.wait() )
      throw util::SystemError("OutputMessageProcessor::waitingForHandler::: call to EventMonitor::wait failed");
  }

  _eventWasSignalled = false;

  if ( _isStopped )
    return NULL;
  else
    return _handler;
}

void
SendMissedCallMessageEventHandler::handle() {
  formOutputMessageAndSendIt(_calledAbnt);
}

void
SendMissedCallMessageEventHandler::formOutputMessageAndSendIt(const AbntAddr& abnt)
{
  smsc_log_debug(_logger, "SendMissedCallMessageEventHandler::formOutputMessageAndSendIt::: format message for subscriber '%s'", abnt.getText().c_str());
  _taskProcessor.ProcessAbntEvents(abnt, this);
}

bool
SendMessageEventHandler::sendBannerRequest(const AbntAddr& abnt,
                                           bool need_banner_in_translit,
                                           MCEventOut* mc_event_out,
                                           uint32_t max_banner_size)
{
  smsc_log_info(_logger, "send BannerRequest to BERotator for subscriber '%s'", abnt.getText().c_str());
  try {
    return _advertising->sendBannerRequest(abnt.toString(), _taskProcessor.getSvcTypeForBE(),
                                           SMPP_SMS, (need_banner_in_translit ? ASCII_TRANSLIT : UTF16BE),
                                           max_banner_size, mc_event_out);
  } catch (NetworkException& ex) {
    smsc_log_error(_logger, "SendMessageEventHandler::sendBannerRequest::: caught NetworkException '%s'", ex.what());
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  } catch (UnrecoveredProtocolError& ex) {
    smsc_log_error(_logger, "SendMessageEventHandler::sendBannerRequest::: caught UnrecoveredProtocolError");
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  } catch (BE_v0_UnsupportedCharsetException& ex) {
    smsc_log_info(_logger, "SendMessageEventHandler::sendBannerRequest::: caught BE_v0_UnsupportedCharsetException: [%s]", ex.what());
  }

  return true;
}

void
SendMessageEventHandler::rollbackBanner(const BannerResponseTrace& bannerRespTrace)
{
  smsc_log_info(_logger, "rollback banner [transactionId=%d,bannerId=%d,ownerId=%d,rotatorId=%d]", bannerRespTrace.transactionId, bannerRespTrace.bannerId, bannerRespTrace.ownerId, bannerRespTrace.rotatorId);
  try {
    _advertising->rollbackBanner(bannerRespTrace.transactionId, bannerRespTrace.bannerId, bannerRespTrace.ownerId, bannerRespTrace.rotatorId);
  } catch (NetworkException& ex) {
    smsc_log_error(_logger, "SendMessageEventHandler::rollbackBanner::: caught NetworkException '%s'", ex.what());
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  } catch (UnrecoveredProtocolError& ex) {
    smsc_log_error(_logger, "SendMessageEventHandler::rollbackBanner::: caught UnrecoveredProtocolError");
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  }

}

void
SendAbonentOnlineNotificationEventHandler::handle()
{
  _taskProcessor.SendAbntOnlineNotifications(_pInfo, _abntProfile, this);
}

void
RollbackBERequestHandler::handle()
{
  rollbackBanner(_bannerRespTrace);
}

}}
