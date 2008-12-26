#ifndef __INMAN_USS_EMULATOR_STRATEGYSENDPOSITIVETRANSLITERATEDMESSAGE_HPP__
# define __INMAN_USS_EMULATOR_STRATEGYSENDPOSITIVETRANSLITERATEDMESSAGE_HPP__

# include <vector>
# include <sys/types.h>
# include <core/threads/ThreadedTask.hpp>
# include <inman/uss/ussmessages.hpp>
# include <inman/uss/emulator/MessageHandler.hpp>
# include <inman/uss/emulator/ResponseWriter.hpp>

namespace smsc  {
namespace inman {
namespace uss  {

class Strategy_SendPositiveTransliteratedMessage : public MessageHandler {
public:
  explicit Strategy_SendPositiveTransliteratedMessage(ResponseWriter* responseWriter)
    : MessageHandler(responseWriter) {}

  virtual void handle();
private:
  std::vector<uint8_t> prepareStringInUCS(const char* textString);
};

}}}

#endif
