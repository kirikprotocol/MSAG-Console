#include "HideOptions.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

const int8_t HideOptions::HideDisabled;
const int8_t HideOptions::HideEnabled;
const int8_t HideOptions::HideSubstitute;
std::map<HideOptions::type,std::string> HideOptions::nameByValue;
std::map<std::string,HideOptions::type> HideOptions::valueByName;
HideOptions::StaticInitializer HideOptions::staticInitializer;

}
}
}
}
}
