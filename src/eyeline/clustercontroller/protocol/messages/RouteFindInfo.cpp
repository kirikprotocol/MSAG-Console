#include "RouteFindInfo.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t RouteFindInfo::NotFound=0;
const int8_t RouteFindInfo::FoundEnabled=1;
const int8_t RouteFindInfo::FoundDisabled=2;
std::map<RouteFindInfo::type,std::string> RouteFindInfo::nameByValue;
std::map<std::string,RouteFindInfo::type> RouteFindInfo::valueByName;
RouteFindInfo::StaticInitializer RouteFindInfo::staticInitializer;

}
}
}
}
