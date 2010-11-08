#ifndef __GENERATED_ENUM_DELIVERYSTATUS__
#define __GENERATED_ENUM_DELIVERYSTATUS__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryStatus{
public:
  typedef int8_t type;
 
  DeliveryStatus()
  {
    isNull=true;
  }
  DeliveryStatus(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("DeliveryStatus",value);
    }
    isNull=false;
  }
 
  DeliveryStatus(const DeliveryStatus& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("DeliveryStatus",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const DeliveryStatus& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type Planned=1;
  static const type Active=2;
  static const type Paused=3;
  static const type Cancelled=4;
  static const type Finished=5;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const DeliveryStatus& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"Planned"));
      valueByName.insert(std::map<std::string,type>::value_type("Planned",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"Active"));
      valueByName.insert(std::map<std::string,type>::value_type("Active",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"Paused"));
      valueByName.insert(std::map<std::string,type>::value_type("Paused",3));
      nameByValue.insert(std::map<type,std::string>::value_type(4,"Cancelled"));
      valueByName.insert(std::map<std::string,type>::value_type("Cancelled",4));
      nameByValue.insert(std::map<type,std::string>::value_type(5,"Finished"));
      valueByName.insert(std::map<std::string,type>::value_type("Finished",5));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
