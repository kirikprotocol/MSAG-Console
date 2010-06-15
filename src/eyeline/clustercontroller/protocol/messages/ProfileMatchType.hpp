#ifndef __GENERATED_ENUM_PROFILEMATCHTYPE__
#define __GENERATED_ENUM_PROFILEMATCHTYPE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class ProfileMatchType{
public:
  typedef int8_t type;
  static const type Default;
  static const type Mask;
  static const type Exact;
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"Default"));
      valueByName.insert(std::map<std::string,type>::value_type("Default",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"Mask"));
      valueByName.insert(std::map<std::string,type>::value_type("Mask",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"Exact"));
      valueByName.insert(std::map<std::string,type>::value_type("Exact",2));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
