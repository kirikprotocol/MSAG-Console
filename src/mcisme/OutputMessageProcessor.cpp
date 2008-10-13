#include "OutputMessageProcessor.hpp"
#include <scag/util/encodings/Encodings.h>
#include <scag/exc/SCAGExceptions.h>

#include "Exceptions.hpp"
#include "advert/AdvertisingImpl.h"

namespace smsc {
namespace mcisme {

OutputMessageProcessor::OutputMessageProcessor(TaskProcessor& taskProcessor,
                                               util::config::ConfigView* advertCfg,
                                               OutputMessageProcessorsDispatcher& dispatcher)
  : _taskProcessor(taskProcessor), _isStopped(false), _eventWasSignalled(false),
    _logger(logger::Logger::getInstance("omsgprc")), _messagesProcessorsDispatcher(dispatcher)
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

  try {
    _connectTimeout = advertCfg->getInt("connectTimeout");
    if ( _connectTimeout < 0 )
      throw util::Exception("OutputMessageProcessor::OutputMessageProcessor::: invalid Advertising.connectTimeout value < 0");
  } catch (...) {
    _connectTimeout = 15;
    smsc_log_warn(_logger, "Parameter <MCISme.Advertising.connectTimeout> missed. Default value is '15'.");
  }

  _advertising = new SimpleAdvertisingClient(advertServer, advertPort, advertTimeout);
  try {
    _advertising->init(_connectTimeout);
  } catch (std::exception& ex) {
    smsc_log_error(_logger, "OutputMessageProcessor::OutputMessageProcessor::: advertising client can't be initialized - catched exception '%s'", ex.what());
  }
}

void
OutputMessageProcessor::stop()
{
  _isStopped = true;
  _abonentInfoEventMonitor.notify();
}

int
OutputMessageProcessor::Execute()
{
  while (!_isStopped) {
    try {
      AbntAddr calledAbnt;
      if ( waitForCalledAbonentInfo(&calledAbnt) ) {
        try {
          formOutputMessageAndSendIt(calledAbnt);
        } catch(...) {
          _messagesProcessorsDispatcher.markMessageProcessorAsFree(this);
          throw;
        }
        _messagesProcessorsDispatcher.markMessageProcessorAsFree(this);
      }
    } catch (NetworkException& ex) {
      smsc_log_error(_logger, "OutputMessageProcessor::Execute::: catched NetworkException '%s'", ex.what());
      _advertising->reinit(_connectTimeout);
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "OutputMessageProcessor::Execute::: catched unexpected exception '%s'", ex.what());
    } catch (...) {
      smsc_log_error(_logger, "OutputMessageProcessor::Execute::: catched unexpected exception '...'");
    }
  }

  try {
    delete _advertising; _advertising = 0;
  } catch (...) {}

  _messagesProcessorsDispatcher.deleteMessageProcessor(this);

  return 0;
}

void
OutputMessageProcessor::assignMessageOutputWork(const AbntAddr& calledAbnt)
{
  core::synchronization::MutexGuard synchronize(_abonentInfoEventMonitor);
  _calledAbnt = calledAbnt;
  _eventWasSignalled = true;
  _abonentInfoEventMonitor.notify();
}

bool
OutputMessageProcessor::waitForCalledAbonentInfo(AbntAddr* calledAbnt)
{
  core::synchronization::MutexGuard synchronize(_abonentInfoEventMonitor);

  while ( !_eventWasSignalled ) {
    if ( _abonentInfoEventMonitor.wait() )
      throw smsc::util::SystemError("OutputMessageProcessor::waitForCalledAbonentInfo::: call to EventMonitor::wait failed");
    
  }

  _eventWasSignalled = false;

  if ( _isStopped )
    return false;
  else {
    *calledAbnt = _calledAbnt;
    return true;
  }
}

void
OutputMessageProcessor::formOutputMessageAndSendIt(const AbntAddr& abnt)
{
  smsc_log_debug(_logger, "OutputMessageProcessor::formOutputMessageAndSendIt::: format message for abonent '%s'", abnt.getText().c_str());
  _taskProcessor.ProcessAbntEvents(abnt, this);
}

string
OutputMessageProcessor::getBanner(const AbntAddr& abnt)
{
  string banner, ret, ret1;
  int rc;

  smsc_log_debug(_logger, "OutputMessageProcessor::getBanner::: call to BE for abonent '%s'", abnt.getText().c_str());
  rc = _advertising->getBanner(abnt.toString(), _taskProcessor.getSvcType(), SMPP_SMS, UTF16BE, ret);
  if(rc == 0)
  {
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
    smsc_log_debug(_logger, "rc = %d; Banner: %s (%s)", rc, banner.c_str(), ret.c_str());
  }
  else
    smsc_log_debug(_logger, "getBanner Error. Error code = %d", rc);

  return banner;
}

}}
