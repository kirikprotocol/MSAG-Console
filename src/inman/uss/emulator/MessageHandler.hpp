#ifndef __INMAN_USS_EMULATOR_MESSAGEHANDLER_HPP__
# define __INMAN_USS_EMULATOR_MESSAGEHANDLER_HPP__

# include <logger/Logger.h>
# include <inman/interaction/connect.hpp>
# include <inman/uss/ussmessages.hpp>
# include <inman/uss/emulator/ResponseWriter.hpp>

namespace smsc  {
namespace inman {
namespace uss  {

class MessageHandler /*: public core::threads::ThreadedTask */ {
public:
  explicit MessageHandler(ResponseWriter* responseWriter=NULL)
    : _logger(logger::Logger::getInstance("msg_hndl")), _responseWriter(responseWriter) {}

  void assignRequest(uint32_t dialogId, const interaction::USSRequestMessage& requestObject,
                     inman::interaction::Connect* conn) {
    _dialogId = dialogId;
    _requestObject = requestObject;
    _conn = conn;
  }

  void cancelRequestProcessing(unsigned connId) {
    smsc_log_debug(_logger, "MessageHandler::cancelRequestProcessing::: try cancel processing on connection=[%d]", connId);
    if ( _responseWriter )
      _responseWriter->cancelScheduledResponse(connId);
  }

  virtual void handle() = 0;
protected:
  logger::Logger* _logger;
  ResponseWriter* _responseWriter;
  uint32_t _dialogId;
  interaction::USSRequestMessage _requestObject;
  inman::interaction::Connect* _conn;
};

}}}

#endif
