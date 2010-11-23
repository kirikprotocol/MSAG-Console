#ifndef __GENERATED_ENUM_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_PROFILEMATCHTYPE__
#define __GENERATED_ENUM_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_PROFILEMATCHTYPE__

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
 
  ProfileMatchType()
  {
    isNull=true;
  }
  ProfileMatchType(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("ProfileMatchType",value);
    }
    isNull=false;
  }
 
  ProfileMatchType(const ProfileMatchType& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("ProfileMatchType",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const ProfileMatchType& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type Default=0;
  static const type Mask=1;
  static const type Exact=2;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const ProfileMatchType& value)
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
