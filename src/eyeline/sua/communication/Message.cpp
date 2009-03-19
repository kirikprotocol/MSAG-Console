#include "Message.hpp"

namespace eyeline {
namespace sua {
namespace communication {

unsigned int Message::_MSG_IDX;

unsigned int
Message::generateMessageIndexValue()
{
  return ++_MSG_IDX;
}

MessageHandlingDispatcherIface*
Message::getHandlingDispatcher()
{
  return NULL;
}

}}}
