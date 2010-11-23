#ifndef __GENERATED_ENUM_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_HIDEOPTIONS__
#define __GENERATED_ENUM_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_HIDEOPTIONS__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class HideOptions{
public:
  typedef int8_t type;
 
  HideOptions()
  {
    isNull=true;
  }
  HideOptions(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("HideOptions",value);
    }
    isNull=false;
  }
 
  HideOptions(const HideOptions& argRhs):value(argRhs.value),isNull(argRhs.isNull)
  {
  }
 
  const type& getValue()const
  {
    return value;
  }
  bool isNullValue()const
  {
    return isNull;
  }
 
  void setValue(const type& argValue)
  {
    if(!isValidValue(argValue))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("HideOptions",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const HideOptions& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type HideDisabled=0;
  static const type HideEnabled=1;
  static const type HideSubstitute=2;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const HideOptions& value)
  {
    std::map<type,std::string>::iterator it=nameByValue.find(value.value);
    if(it!=nameByValue.end())
    {
      return it->second;
    }
    return "";
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
  type value;
  bool isNull;

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


#endif
