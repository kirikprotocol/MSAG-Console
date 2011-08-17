#include "SmeBindMode.hpp"

namespace eyeline{
namespace load_balancer{
namespace cluster{
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
