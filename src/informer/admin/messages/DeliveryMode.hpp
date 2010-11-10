#ifndef __GENERATED_ENUM_EYELINE_INFORMER_ADMIN_MESSAGES_DELIVERYMODE__
#define __GENERATED_ENUM_EYELINE_INFORMER_ADMIN_MESSAGES_DELIVERYMODE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace informer{
namespace admin{
namespace messages{


class DeliveryMode{
public:
  typedef int8_t type;
 
  DeliveryMode()
  {
    isNull=true;
  }
  DeliveryMode(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("DeliveryMode",value);
    }
    isNull=false;
  }
 
  DeliveryMode(const DeliveryMode& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("DeliveryMode",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const DeliveryMode& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type SMS=1;
  static const type USSD_PUSH=2;
  static const type USSD_PUSH_VLR=3;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const DeliveryMode& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"SMS"));
      valueByName.insert(std::map<std::string,type>::value_type("SMS",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"USSD_PUSH"));
      valueByName.insert(std::map<std::string,type>::value_type("USSD_PUSH",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"USSD_PUSH_VLR"));
      valueByName.insert(std::map<std::string,type>::value_type("USSD_PUSH_VLR",3));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
