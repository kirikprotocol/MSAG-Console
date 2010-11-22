#include "Status.hpp"

namespace smsc{
namespace mcisme{
namespace mcaia{

const int8_t Status::OK;
const int8_t Status::InvalidCalledAddress;
const int8_t Status::ExecutingError;
const int8_t Status::Timedout;
const int8_t Status::Rejected;
std::map<Status::type,std::string> Status::nameByValue;
std::map<std::string,Status::type> Status::valueByName;
Status::StaticInitializer Status::staticInitializer;

}
}
}
