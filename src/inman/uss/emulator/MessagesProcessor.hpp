#ifndef __INMAN_USS_EMULATOR_MESSAGESPROCESSOR_HPP__
# define __INMAN_USS_EMULATOR_MESSAGESPROCESSOR_HPP__

# include <logger/Logger.h>
# include <util/Singleton.hpp>
# include <util/config/ConfigView.h>
# include <core/threads/ThreadPool.hpp>

# include <inman/interaction/connect.hpp>

# include <inman/uss/ussmessages.hpp>
# include <inman/uss/emulator/MessageHandler.hpp>
# include <inman/uss/emulator/ResponseWriter.hpp>
# include <inman/uss/emulator/RequestProcessingRules.hpp>

namespace smsc  {
namespace inman {
namespace uss  {

class MessagesProcessor : public util::Singleton<MessagesProcessor> {
public:
  MessagesProcessor();
  void init(util::config::ConfigView& cfg);
  MessageHandler* processRequest(uint32_t dialogId,
                                 const interaction::USSRequestMessage& requestObject,
                                 inman::interaction::Connect* conn);
private:
  MessageHandler* getNextMessageHandler();
  logger::Logger* _logger;
  core::threads::ThreadPool _threadPool;
  inman::interaction::Connect* _conn;

  typedef ResponseWriter* ResponseWriterPtr_t;

  ResponseWriterPtr_t _oneSecondDelayQueue, _twoSecondsDelayQueue,
    _fourSecondsDelayQueue, _defaultDelayQueue;

  RequestProcessingRules _procRules;
};

}}}

#endif
