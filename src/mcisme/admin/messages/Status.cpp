#include "Status.hpp"

namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{

const int8_t Status::OK;
const int8_t Status::SystemError;
const int8_t Status::SubscriberNotFound;
std::map<Status::type,std::string> Status::nameByValue;
std::map<std::string,Status::type> Status::valueByName;
Status::StaticInitializer Status::staticInitializer;

}
}
}
}
