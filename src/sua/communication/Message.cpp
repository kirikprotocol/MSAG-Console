#include "Message.hpp"

unsigned int communication::Message::_MSG_IDX;

unsigned int
communication::Message::generateMessageIndexValue()
{
  return ++_MSG_IDX;
}
