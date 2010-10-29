#include "ConfigOpId.hpp"

namespace eyeline{
namespace informer{
namespace admin{
namespace messages{

const int8_t ConfigOpId::coAdd;
const int8_t ConfigOpId::coRemove;
const int8_t ConfigOpId::coUpdate;
std::map<ConfigOpId::type,std::string> ConfigOpId::nameByValue;
std::map<std::string,ConfigOpId::type> ConfigOpId::valueByName;
ConfigOpId::StaticInitializer ConfigOpId::staticInitializer;

}
}
}
}
