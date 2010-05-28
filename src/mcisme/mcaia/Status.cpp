#include "Status.hpp"

namespace smsc{
namespace mcisme{
namespace mcaia{

const int8_t Status::OK=0;
const int8_t Status::InvalidCalledAddress=1;
const int8_t Status::ExecutingError=2;
const int8_t Status::Timedout=3;
const int8_t Status::Rejected=4;
std::map<Status::type,std::string> Status::nameByValue;
std::map<std::string,Status::type> Status::valueByName;
Status::StaticInitializer Status::staticInitializer;

}
}
}
