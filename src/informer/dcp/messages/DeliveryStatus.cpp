#include "DeliveryStatus.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{

const int8_t DeliveryStatus::Planned;
const int8_t DeliveryStatus::Active;
const int8_t DeliveryStatus::Paused;
const int8_t DeliveryStatus::Cancelled;
const int8_t DeliveryStatus::Finished;
std::map<DeliveryStatus::type,std::string> DeliveryStatus::nameByValue;
std::map<std::string,DeliveryStatus::type> DeliveryStatus::valueByName;
DeliveryStatus::StaticInitializer DeliveryStatus::staticInitializer;

}
}
}
}
