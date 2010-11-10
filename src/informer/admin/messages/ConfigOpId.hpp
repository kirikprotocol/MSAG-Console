#ifndef __GENERATED_ENUM_EYELINE_INFORMER_ADMIN_MESSAGES_CONFIGOPID__
#define __GENERATED_ENUM_EYELINE_INFORMER_ADMIN_MESSAGES_CONFIGOPID__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class ConfigOpId{
public:
  typedef int8_t type;
 
  ConfigOpId()
  {
    isNull=true;
  }
  ConfigOpId(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("ConfigOpId",value);
    }
    isNull=false;
  }
 
  ConfigOpId(const ConfigOpId& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("ConfigOpId",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const ConfigOpId& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type coAdd=1;
  static const type coRemove=2;
  static const type coUpdate=3;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const ConfigOpId& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"coAdd"));
      valueByName.insert(std::map<std::string,type>::value_type("coAdd",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"coRemove"));
      valueByName.insert(std::map<std::string,type>::value_type("coRemove",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"coUpdate"));
      valueByName.insert(std::map<std::string,type>::value_type("coUpdate",3));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
