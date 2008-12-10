#include "OutputMessageProcessor.hpp"
#include <scag/util/encodings/Encodings.h>
#include <scag/exc/SCAGExceptions.h>

#include "Exceptions.hpp"
#include "advert/BEProtocolV1SimpleClient.hpp"
#include "advert/BEProtocolV2SimpleClient.hpp"
#include "advert/BEReconnector.hpp"

namespace smsc {
namespace mcisme {

OutputMessageProcessor::OutputMessageProcessor(TaskProcessor& taskProcessor,
                                               util::config::ConfigView* advertCfg,
                                               OutputMessageProcessorsDispatcher& dispatcher,
                                               BEReconnector& reconnectorThread)
  : _taskProcessor(taskProcessor), _isStopped(false), _eventWasSignalled(false),
    _logger(logger::Logger::getInstance("omsgprc")), _handler(NULL),
    _messagesProcessorsDispatcher(dispatcher), _reconnectorThread(reconnectorThread)
{
  if(!advertCfg->getBool("useAdvert"))
    throw util::Exception("OutputMessageProcessor::OutputMessageProcessor::: Fatal! Advertising.useAdvert = false");

  string advertServer;
  try {
    advertServer = advertCfg->getString("server");
  } catch (...) {
    advertServer = "0.0.0.0";
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.server> missed. Default value is '0.0.0.0'.");
  }
  int advertPort;
  try {
    advertPort = advertCfg->getInt("port");
  } catch (...){
    advertPort = 25000;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.port> missed. Default value is '25000'.");
  }
  int advertTimeout;
  try {
    advertTimeout = advertCfg->getInt("timeout");
    if ( advertTimeout < 0 )
      throw util::Exception("OutputMessageProcessor::OutputMessageProcessor::: invalid Advertising.timeout value < 0");
  } catch (...) {
    advertTimeout = 15;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.timeout> missed. Default value is '15'.");
  }

  int connectTimeout;
  try {
    connectTimeout = advertCfg->getInt("connectTimeout");
    if ( connectTimeout < 0 )
      throw util::Exception("OutputMessageProcessor::OutputMessageProcessor::: invalid Advertising.connectTimeout value < 0");
  } catch (...) {
    connectTimeout = 15;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.connectTimeout> missed. Default value is '15'.");
  }

  try {
    int bannerEngineProtocolVersion = advertCfg->getInt("BEProtocolVersion");
    switch (bannerEngineProtocolVersion) {
    case BEProtocolV1SimpleClient::PROTOCOL_VERSION:
      _advertising = new BEProtocolV1SimpleClient(advertServer, advertPort, advertTimeout);
      break;
    case BEProtocolV2SimpleClient::PROTOCOL_VERSION:
      _advertising = new BEProtocolV2SimpleClient(advertServer, advertPort, advertTimeout);
      break;
    default:
      throw util::Exception("OutputMessageProcessor::OutputMessageProcessor::: invalid Advertising.BEProtocolVersion value [%d]", bannerEngineProtocolVersion);
    }
  } catch (ConfigException& ex) {
    _advertising = new BEProtocolV1SimpleClient(advertServer, advertPort, advertTimeout);
  }

  try {
    _advertising->init(connectTimeout);
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "OutputMessageProcessor::OutputMessageProcessor::: advertising client can't be initialized - catched exception '%s'", ex.what());
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  }
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
      smsc_log_error(_logger, "OutputMessageProcessor::Execute::: catched unexpected exception '%s'", ex.what());
    } catch (...) {
      smsc_log_error(_logger, "OutputMessageProcessor::Execute::: catched unexpected exception '...'");
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
  smsc_log_debug(_logger, "SendMissedCallMessageEventHandler::formOutputMessageAndSendIt::: format message for abonent '%s'", abnt.getText().c_str());
  _taskProcessor.ProcessAbntEvents(abnt, this);
}

string
SendMessageEventHandler::getBanner(const AbntAddr& abnt, BannerResponseTrace* bannerRespTrace, bool needBannerInTranslit)
{
  string banner, ret, ret1;

  smsc_log_debug(_logger, "SendMessageEventHandler::getBanner::: call to BE for abonent '%s'", abnt.getText().c_str());
  try {
    uint32_t bannerCharSet = UTF16BE;
    if ( needBannerInTranslit )
      bannerCharSet = ASCII_TRANSLIT;
    int rc = _advertising->getBanner(abnt.toString(), _taskProcessor.getSvcTypeForBE(), SMPP_SMS, bannerCharSet, &ret, bannerRespTrace);
    if(rc == 0)
    {
      if ( bannerCharSet == UTF16BE ) {
        try {
          scag::util::encodings::Convertor::convert("UTF-16BE", "UTF-8", ret.c_str(), ret.length(), ret1);
        } catch(scag::exceptions::SCAGException e) {
          smsc_log_error(_logger, "Exc: %s", e.what());
          return banner="";
        }
        try {
          scag::util::encodings::Convertor::convert("UTF-8", "CP1251", ret1.c_str(), ret1.length(), banner);
        } catch(scag::exceptions::SCAGException e) {
          smsc_log_error(_logger, "Exc: %s", e.what());
          return banner="";
        }
      } else 
        banner = ret;
    }
    else
      smsc_log_debug(_logger, "getBanner Error. Error code = %d", rc);
  } catch (NetworkException& ex) {
    smsc_log_error(_logger, "SendMessageEventHandler::getBanner::: catched NetworkException '%s'", ex.what());
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  } catch (UnrecoveredProtocolError& ex) {
    smsc_log_error(_logger, "SendMessageEventHandler::getBanner::: catched UnrecoveredProtocolError");
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  }

  return banner;
}

void
SendMessageEventHandler::rollbackBanner(const BannerResponseTrace& bannerRespTrace)
{
  smsc_log_debug(_logger, "SendMessageEventHandler::rollbackBanner::: rollback banner [transactionId=%d,bannerId=%d,ownerId=%d,rotatorId=%d]", bannerRespTrace.transactionId, bannerRespTrace.bannerId, bannerRespTrace.ownerId, bannerRespTrace.rotatorId);
  try {
    _advertising->rollbackBanner(bannerRespTrace.transactionId, bannerRespTrace.bannerId, bannerRespTrace.ownerId, bannerRespTrace.rotatorId);
  } catch (NetworkException& ex) {
    smsc_log_error(_logger, "SendMessageEventHandler::rollbackBanner::: catched NetworkException '%s'", ex.what());
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  } catch (UnrecoveredProtocolError& ex) {
    smsc_log_error(_logger, "SendMessageEventHandler::rollbackBanner::: catched UnrecoveredProtocolError");
    _reconnectorThread.scheduleBrokenConnectionToReestablishing(_advertising);
  }

}

void
SendAbonentOnlineNotificationEventHandler::handle()
{
  _taskProcessor.SendAbntOnlineNotifications(_pInfo, _abntProfile, this);
  _taskProcessor.commitMissedCallEvents(_pInfo, _abntProfile);
}

void
RollbackBERequestHandler::handle()
{
  rollbackBanner(_bannerRespTrace);
}

}}
