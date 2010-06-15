#ifndef __GENERATED_ENUM_ACLCACHETYPE__
#define __GENERATED_ENUM_ACLCACHETYPE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class AclCacheType{
public:
  typedef int8_t type;
  static const type cacheNone;
  static const type cacheDb;
  static const type cacheFull;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }
 
  static std::string getNameByValue(const type& value)
  {
    std::map<type,std::string>::iterator it=nameByValue.find(value);
    if(it!=nameByValue.end())
    {
      return it->second;
    }
    return "";
  }
 
  static type getValueByName(const std::string& name)
  {
    std::map<std::string,type>::iterator it=valueByName.find(name);
    if(it!=valueByName.end())
    {
      return it->second;
    }
    return type();
  }
 
 
protected:
  static std::map<type,std::string> nameByValue;
  static std::map<std::string,type> valueByName;
  struct StaticInitializer{
    StaticInitializer()
    {
      nameByValue.insert(std::map<type,std::string>::value_type(49,"cacheNone"));
      valueByName.insert(std::map<std::string,type>::value_type("cacheNone",49));
      nameByValue.insert(std::map<type,std::string>::value_type(50,"cacheDb"));
      valueByName.insert(std::map<std::string,type>::value_type("cacheDb",50));
      nameByValue.insert(std::map<type,std::string>::value_type(51,"cacheFull"));
      valueByName.insert(std::map<std::string,type>::value_type("cacheFull",51));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}
}


#endif
