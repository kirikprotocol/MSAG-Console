#ifndef __GENERATED_ENUM_SMECONNECTTYPE__
#define __GENERATED_ENUM_SMECONNECTTYPE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class SmeConnectType{
public:
  typedef int8_t type;
  static const type loadBalancer;
  static const type directConnect;
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"loadBalancer"));
      valueByName.insert(std::map<std::string,type>::value_type("loadBalancer",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"directConnect"));
      valueByName.insert(std::map<std::string,type>::value_type("directConnect",2));
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
