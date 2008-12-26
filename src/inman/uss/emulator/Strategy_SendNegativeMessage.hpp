#ifndef __INMAN_USS_EMULATOR_STRATEGYSENDNEGATIVEMESSAGE_HPP__
# define __INMAN_USS_EMULATOR_STRATEGYSENDNEGATIVEMESSAGE_HPP__

# include <core/threads/ThreadedTask.hpp>
# include <inman/uss/ussmessages.hpp>
# include <inman/uss/emulator/MessageHandler.hpp>
# include <inman/uss/emulator/ResponseWriter.hpp>

namespace smsc  {
namespace inman {
namespace uss  {

class Strategy_SendNegativeMessage : public MessageHandler {
public:
  explicit Strategy_SendNegativeMessage(ResponseWriter* responseWriter)
    : MessageHandler(responseWriter) {}

  virtual void handle();
};

}}}

#endif
