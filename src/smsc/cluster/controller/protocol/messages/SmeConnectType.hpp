#ifndef __GENERATED_ENUM_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMECONNECTTYPE__
#define __GENERATED_ENUM_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_SMECONNECTTYPE__

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
 
  SmeConnectType()
  {
    isNull=true;
  }
  SmeConnectType(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("SmeConnectType",value);
    }
    isNull=false;
  }
 
  SmeConnectType(const SmeConnectType& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("SmeConnectType",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const SmeConnectType& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type loadBalancer=1;
  static const type directConnect=2;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const SmeConnectType& value)
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
