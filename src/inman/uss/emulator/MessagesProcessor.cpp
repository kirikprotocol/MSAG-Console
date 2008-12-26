#include <memory>
#include <stdlib.h>
#include "MessagesProcessor.hpp"
#include "MessageHandler.hpp"
#include "Strategy_DropMessage.hpp"
#include "Strategy_SendNegativeMessage.hpp"
#include "Strategy_SendPositiveTransliteratedMessage.hpp"
#include "ResponseWriter.hpp"

namespace smsc  {
namespace inman {
namespace uss  {

MessagesProcessor::MessagesProcessor()
  : _logger(logger::Logger::getInstance("msgs_proc"))
{}

void
MessagesProcessor::init(util::config::ConfigView& cfg/*'RequestProcessingRules' config's section */)
{
  _procRules.init(cfg);

  smsc_log_debug(_logger, "MessagesProcessor::init::: request processing rules=[%s]", _procRules.toString().c_str());
  _oneSecondDelayQueue = new ResponseWriter(1000);
  _twoSecondsDelayQueue = new ResponseWriter(2000);
  _fourSecondsDelayQueue = new ResponseWriter(4000);
  _defaultDelayQueue = new ResponseWriter(_procRules.mandatoryDelay);

  _oneSecondDelayQueue->Start();
  _twoSecondsDelayQueue->Start();
  _fourSecondsDelayQueue->Start();
  _defaultDelayQueue->Start();
}

MessageHandler*
MessagesProcessor::processRequest(uint32_t dialogId,
                                  const interaction::USSRequestMessage& requestObject,
                                  inman::interaction::Connect* conn)
{
  std::auto_ptr<MessageHandler> requestWorker(getNextMessageHandler());
  requestWorker->assignRequest(dialogId, requestObject, conn);

  requestWorker->handle();

  return requestWorker.release();
}

MessageHandler*
MessagesProcessor::getNextMessageHandler()
{
  int randomValue = rand();
  if ( randomValue >=0 && randomValue < _procRules.oneSecondDelayProbability )
    return new Strategy_SendPositiveTransliteratedMessage(_oneSecondDelayQueue);
  else if ( randomValue >= _procRules.oneSecondDelayProbability &&
            randomValue < _procRules.oneSecondDelayProbability + _procRules.twoSecondsDelayProbability )
    return new Strategy_SendPositiveTransliteratedMessage(_twoSecondsDelayQueue);
  else if ( randomValue >= _procRules.oneSecondDelayProbability + _procRules.twoSecondsDelayProbability &&
            randomValue < _procRules.oneSecondDelayProbability + _procRules.twoSecondsDelayProbability + _procRules.fourSecondsDelayProbability)
    return new Strategy_SendPositiveTransliteratedMessage(_fourSecondsDelayQueue);
  else if ( randomValue >= _procRules.oneSecondDelayProbability + _procRules.twoSecondsDelayProbability + _procRules.fourSecondsDelayProbability &&
            randomValue < _procRules.oneSecondDelayProbability + _procRules.twoSecondsDelayProbability + _procRules.fourSecondsDelayProbability + _procRules.rejectProbability)
    return new Strategy_DropMessage();
  else if ( randomValue >= _procRules.oneSecondDelayProbability + _procRules.twoSecondsDelayProbability + _procRules.fourSecondsDelayProbability + _procRules.rejectProbability &&
            randomValue < _procRules.oneSecondDelayProbability + _procRules.twoSecondsDelayProbability + _procRules.fourSecondsDelayProbability + _procRules.rejectProbability + _procRules.nonOkStatusProbability )
    return new Strategy_SendNegativeMessage(_defaultDelayQueue);
  else
    return new Strategy_SendPositiveTransliteratedMessage(_defaultDelayQueue);
}

}}}
