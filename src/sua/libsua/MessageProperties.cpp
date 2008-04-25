#include "MessageProperties.hpp"

namespace libsua {

MessageProperties::MessageProperties()
  :  protocolClass(0), returnOnError(false), sequenceControlValue(0),
     importance(0), messagePriority(0), correlationId(0), hopCount(0), fieldsMask(0)
{}

}
