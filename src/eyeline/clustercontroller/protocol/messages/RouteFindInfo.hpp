#ifndef __GENERATED_ENUM_ROUTEFINDINFO__
#define __GENERATED_ENUM_ROUTEFINDINFO__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class RouteFindInfo{
public:
  typedef int8_t type;
  static const type NotFound;
  static const type FoundEnabled;
  static const type FoundDisabled;
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"NotFound"));
      valueByName.insert(std::map<std::string,type>::value_type("NotFound",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"FoundEnabled"));
      valueByName.insert(std::map<std::string,type>::value_type("FoundEnabled",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"FoundDisabled"));
      valueByName.insert(std::map<std::string,type>::value_type("FoundDisabled",2));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
