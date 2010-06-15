#ifndef __GENERATED_ENUM_SMEBINDMODE__
#define __GENERATED_ENUM_SMEBINDMODE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class SmeBindMode{
public:
  typedef int8_t type;
  static const type modeTx;
  static const type modeRx;
  static const type modeTrx;
  static const type modeUnknown;
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"modeTx"));
      valueByName.insert(std::map<std::string,type>::value_type("modeTx",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"modeRx"));
      valueByName.insert(std::map<std::string,type>::value_type("modeRx",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"modeTrx"));
      valueByName.insert(std::map<std::string,type>::value_type("modeTrx",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"modeUnknown"));
      valueByName.insert(std::map<std::string,type>::value_type("modeUnknown",3));
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
