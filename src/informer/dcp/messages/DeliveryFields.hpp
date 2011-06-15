#ifndef __GENERATED_ENUM_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYFIELDS__
#define __GENERATED_ENUM_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYFIELDS__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryFields{
public:
  typedef int8_t type;
 
  DeliveryFields()
  {
    isNull=true;
  }
  DeliveryFields(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("DeliveryFields",value);
    }
    isNull=false;
  }
 
  DeliveryFields(const DeliveryFields& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("DeliveryFields",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const DeliveryFields& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type UserId=1;
  static const type Name=2;
  static const type Status=3;
  static const type StartDate=4;
  static const type EndDate=5;
  static const type ActivityPeriod=6;
  static const type UserData=7;
  static const type CreationDate=8;
  static const type BoundToLocalTime=9;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const DeliveryFields& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"UserId"));
      valueByName.insert(std::map<std::string,type>::value_type("UserId",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"Name"));
      valueByName.insert(std::map<std::string,type>::value_type("Name",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"Status"));
      valueByName.insert(std::map<std::string,type>::value_type("Status",3));
      nameByValue.insert(std::map<type,std::string>::value_type(4,"StartDate"));
      valueByName.insert(std::map<std::string,type>::value_type("StartDate",4));
      nameByValue.insert(std::map<type,std::string>::value_type(5,"EndDate"));
      valueByName.insert(std::map<std::string,type>::value_type("EndDate",5));
      nameByValue.insert(std::map<type,std::string>::value_type(6,"ActivityPeriod"));
      valueByName.insert(std::map<std::string,type>::value_type("ActivityPeriod",6));
      nameByValue.insert(std::map<type,std::string>::value_type(7,"UserData"));
      valueByName.insert(std::map<std::string,type>::value_type("UserData",7));
      nameByValue.insert(std::map<type,std::string>::value_type(8,"CreationDate"));
      valueByName.insert(std::map<std::string,type>::value_type("CreationDate",8));
      nameByValue.insert(std::map<type,std::string>::value_type(9,"BoundToLocalTime"));
      valueByName.insert(std::map<std::string,type>::value_type("BoundToLocalTime",9));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
