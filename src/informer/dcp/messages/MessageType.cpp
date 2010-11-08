#include "MessageType.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{

const int8_t MessageType::TextMessage;
const int8_t MessageType::GlossaryMessage;
std::map<MessageType::type,std::string> MessageType::nameByValue;
std::map<std::string,MessageType::type> MessageType::valueByName;
MessageType::StaticInitializer MessageType::staticInitializer;

}
}
}
}
