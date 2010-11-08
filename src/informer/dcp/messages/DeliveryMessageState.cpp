#include "DeliveryMessageState.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{

const int8_t DeliveryMessageState::New;
const int8_t DeliveryMessageState::Process;
const int8_t DeliveryMessageState::Delivered;
const int8_t DeliveryMessageState::Failed;
const int8_t DeliveryMessageState::Expired;
std::map<DeliveryMessageState::type,std::string> DeliveryMessageState::nameByValue;
std::map<std::string,DeliveryMessageState::type> DeliveryMessageState::valueByName;
DeliveryMessageState::StaticInitializer DeliveryMessageState::staticInitializer;

}
}
}
}
