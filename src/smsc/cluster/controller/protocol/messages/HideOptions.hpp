#ifndef __GENERATED_ENUM_HIDEOPTIONS__
#define __GENERATED_ENUM_HIDEOPTIONS__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class HideOptions{
public:
  typedef int8_t type;
  static const type HideDisabled;
  static const type HideEnabled;
  static const type HideSubstitute;
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"HideDisabled"));
      valueByName.insert(std::map<std::string,type>::value_type("HideDisabled",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"HideEnabled"));
      valueByName.insert(std::map<std::string,type>::value_type("HideEnabled",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"HideSubstitute"));
      valueByName.insert(std::map<std::string,type>::value_type("HideSubstitute",2));
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
