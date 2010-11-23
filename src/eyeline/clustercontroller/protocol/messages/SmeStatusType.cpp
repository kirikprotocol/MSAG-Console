#include "SmeStatusType.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t SmeStatusType::disconnected;
const int8_t SmeStatusType::bound;
const int8_t SmeStatusType::internal;
const int8_t SmeStatusType::unknown;
std::map<SmeStatusType::type,std::string> SmeStatusType::nameByValue;
std::map<std::string,SmeStatusType::type> SmeStatusType::valueByName;
SmeStatusType::StaticInitializer SmeStatusType::staticInitializer;

}
}
}
}
