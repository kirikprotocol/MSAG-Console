#include "SmeBindMode.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

const int8_t SmeBindMode::modeTx;
const int8_t SmeBindMode::modeRx;
const int8_t SmeBindMode::modeTrx;
const int8_t SmeBindMode::modeUnknown;
std::map<SmeBindMode::type,std::string> SmeBindMode::nameByValue;
std::map<std::string,SmeBindMode::type> SmeBindMode::valueByName;
SmeBindMode::StaticInitializer SmeBindMode::staticInitializer;

}
}
}
}
}
