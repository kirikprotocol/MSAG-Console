#include "ProfileMatchType.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t ProfileMatchType::Default=0;
const int8_t ProfileMatchType::Mask=1;
const int8_t ProfileMatchType::Exact=2;
std::map<ProfileMatchType::type,std::string> ProfileMatchType::nameByValue;
std::map<std::string,ProfileMatchType::type> ProfileMatchType::valueByName;
ProfileMatchType::StaticInitializer ProfileMatchType::staticInitializer;

}
}
}
}
