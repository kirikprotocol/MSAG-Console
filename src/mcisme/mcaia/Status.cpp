#include "Status.hpp"

namespace smsc{
namespace mcisme{
namespace mcaia{

const int8_t Status::OK=0;
const int8_t Status::InvalidCalledAddress=1;
const int8_t Status::QueueFull=2;
std::map<Status::type,std::string> Status::nameByValue;
std::map<std::string,Status::type> Status::valueByName;
Status::StaticInitializer Status::staticInitializer;

}
}
}
