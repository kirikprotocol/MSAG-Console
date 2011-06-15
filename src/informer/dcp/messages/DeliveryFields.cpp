#include "DeliveryFields.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{

const int8_t DeliveryFields::UserId;
const int8_t DeliveryFields::Name;
const int8_t DeliveryFields::Status;
const int8_t DeliveryFields::StartDate;
const int8_t DeliveryFields::EndDate;
const int8_t DeliveryFields::ActivityPeriod;
const int8_t DeliveryFields::UserData;
const int8_t DeliveryFields::CreationDate;
const int8_t DeliveryFields::BoundToLocalTime;
std::map<DeliveryFields::type,std::string> DeliveryFields::nameByValue;
std::map<std::string,DeliveryFields::type> DeliveryFields::valueByName;
DeliveryFields::StaticInitializer DeliveryFields::staticInitializer;

}
}
}
}
