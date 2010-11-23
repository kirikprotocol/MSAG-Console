#include "ProfileMatchType.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t ProfileMatchType::Default;
const int8_t ProfileMatchType::Mask;
const int8_t ProfileMatchType::Exact;
std::map<ProfileMatchType::type,std::string> ProfileMatchType::nameByValue;
std::map<std::string,ProfileMatchType::type> ProfileMatchType::valueByName;
ProfileMatchType::StaticInitializer ProfileMatchType::staticInitializer;

}
}
}
}
