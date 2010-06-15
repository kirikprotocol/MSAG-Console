#include "HideOptions.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

const int8_t HideOptions::HideDisabled=0;
const int8_t HideOptions::HideEnabled=1;
const int8_t HideOptions::HideSubstitute=2;
std::map<HideOptions::type,std::string> HideOptions::nameByValue;
std::map<std::string,HideOptions::type> HideOptions::valueByName;
HideOptions::StaticInitializer HideOptions::staticInitializer;

}
}
}
}
}
