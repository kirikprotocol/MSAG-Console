#include "ProfileCharset.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t ProfileCharset::Default=0;
const int8_t ProfileCharset::Latin1=1;
const int8_t ProfileCharset::Ucs2=8;
const int8_t ProfileCharset::Ucs2AndLatin1=9;
const int8_t ProfileCharset::UssdIn7BitFlag=128;
std::map<ProfileCharset::type,std::string> ProfileCharset::nameByValue;
std::map<std::string,ProfileCharset::type> ProfileCharset::valueByName;
ProfileCharset::StaticInitializer ProfileCharset::staticInitializer;

}
}
}
}
