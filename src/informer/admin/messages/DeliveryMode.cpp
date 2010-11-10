#include "DeliveryMode.hpp"

namespace eyeline{
namespace informer{
namespace admin{
namespace messages{

const int8_t DeliveryMode::SMS;
const int8_t DeliveryMode::USSD_PUSH;
const int8_t DeliveryMode::USSD_PUSH_VLR;
std::map<DeliveryMode::type,std::string> DeliveryMode::nameByValue;
std::map<std::string,DeliveryMode::type> DeliveryMode::valueByName;
DeliveryMode::StaticInitializer DeliveryMode::staticInitializer;

}
}
}
}
