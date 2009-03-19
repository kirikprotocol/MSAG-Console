#include "MessageProperties.hpp"

namespace eyeline {
namespace sua {
namespace libsua {

MessageProperties::MessageProperties()
  :  returnOnError(false), sequenceControlValue(0),
     importance(0), hopCount(0), fieldsMask(0)
{}

}}}
