#ifndef __GENERATED_ENUM_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_CONFIGTYPE__
#define __GENERATED_ENUM_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_CONFIGTYPE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class ConfigType{
public:
  typedef int8_t type;
 
  ConfigType()
  {
    isNull=true;
  }
  ConfigType(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("ConfigType",value);
    }
    isNull=false;
  }
 
  ConfigType(const ConfigType& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("ConfigType",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const ConfigType& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type MainConfig=0;
  static const type Profiles=1;
  static const type Msc=2;
  static const type Routes=3;
  static const type Sme=4;
  static const type ClosedGroups=5;
  static const type Aliases=6;
  static const type MapLimits=7;
  static const type Resources=8;
  static const type Reschedule=9;
  static const type Snmp=10;
  static const type TimeZones=11;
  static const type Fraud=12;
  static const type Acl=13;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const ConfigType& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"MainConfig"));
      valueByName.insert(std::map<std::string,type>::value_type("MainConfig",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"Profiles"));
      valueByName.insert(std::map<std::string,type>::value_type("Profiles",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"Msc"));
      valueByName.insert(std::map<std::string,type>::value_type("Msc",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"Routes"));
      valueByName.insert(std::map<std::string,type>::value_type("Routes",3));
      nameByValue.insert(std::map<type,std::string>::value_type(4,"Sme"));
      valueByName.insert(std::map<std::string,type>::value_type("Sme",4));
      nameByValue.insert(std::map<type,std::string>::value_type(5,"ClosedGroups"));
      valueByName.insert(std::map<std::string,type>::value_type("ClosedGroups",5));
      nameByValue.insert(std::map<type,std::string>::value_type(6,"Aliases"));
      valueByName.insert(std::map<std::string,type>::value_type("Aliases",6));
      nameByValue.insert(std::map<type,std::string>::value_type(7,"MapLimits"));
      valueByName.insert(std::map<std::string,type>::value_type("MapLimits",7));
      nameByValue.insert(std::map<type,std::string>::value_type(8,"Resources"));
      valueByName.insert(std::map<std::string,type>::value_type("Resources",8));
      nameByValue.insert(std::map<type,std::string>::value_type(9,"Reschedule"));
      valueByName.insert(std::map<std::string,type>::value_type("Reschedule",9));
      nameByValue.insert(std::map<type,std::string>::value_type(10,"Snmp"));
      valueByName.insert(std::map<std::string,type>::value_type("Snmp",10));
      nameByValue.insert(std::map<type,std::string>::value_type(11,"TimeZones"));
      valueByName.insert(std::map<std::string,type>::value_type("TimeZones",11));
      nameByValue.insert(std::map<type,std::string>::value_type(12,"Fraud"));
      valueByName.insert(std::map<std::string,type>::value_type("Fraud",12));
      nameByValue.insert(std::map<type,std::string>::value_type(13,"Acl"));
      valueByName.insert(std::map<std::string,type>::value_type("Acl",13));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
