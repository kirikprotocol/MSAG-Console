#include "SmeBindMode.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

const int8_t SmeBindMode::modeTx=0;
const int8_t SmeBindMode::modeRx=1;
const int8_t SmeBindMode::modeTrx=2;
const int8_t SmeBindMode::modeUnknown=3;
std::map<SmeBindMode::type,std::string> SmeBindMode::nameByValue;
std::map<std::string,SmeBindMode::type> SmeBindMode::valueByName;
SmeBindMode::StaticInitializer SmeBindMode::staticInitializer;

}
}
}
}
}
