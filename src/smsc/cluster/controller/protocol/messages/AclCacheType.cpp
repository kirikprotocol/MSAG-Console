#include "AclCacheType.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{

const int8_t AclCacheType::cacheNone=49;
const int8_t AclCacheType::cacheDb=50;
const int8_t AclCacheType::cacheFull=51;
std::map<AclCacheType::type,std::string> AclCacheType::nameByValue;
std::map<std::string,AclCacheType::type> AclCacheType::valueByName;
AclCacheType::StaticInitializer AclCacheType::staticInitializer;

}
}
}
}
}
