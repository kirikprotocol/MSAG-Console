#ifndef __GENERATED_ENUM_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYMESSAGESTATE__
#define __GENERATED_ENUM_EYELINE_INFORMER_DCP_MESSAGES_DELIVERYMESSAGESTATE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class DeliveryMessageState{
public:
  typedef int8_t type;
 
  DeliveryMessageState()
  {
    isNull=true;
  }
  DeliveryMessageState(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("DeliveryMessageState",value);
    }
    isNull=false;
  }
 
  DeliveryMessageState(const DeliveryMessageState& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("DeliveryMessageState",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const DeliveryMessageState& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type New=1;
  static const type Process=2;
  static const type Retry=6;
  static const type Delivered=3;
  static const type Failed=4;
  static const type Expired=5;
  static const type Killed=7;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const DeliveryMessageState& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"New"));
      valueByName.insert(std::map<std::string,type>::value_type("New",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"Process"));
      valueByName.insert(std::map<std::string,type>::value_type("Process",2));
      nameByValue.insert(std::map<type,std::string>::value_type(6,"Retry"));
      valueByName.insert(std::map<std::string,type>::value_type("Retry",6));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"Delivered"));
      valueByName.insert(std::map<std::string,type>::value_type("Delivered",3));
      nameByValue.insert(std::map<type,std::string>::value_type(4,"Failed"));
      valueByName.insert(std::map<std::string,type>::value_type("Failed",4));
      nameByValue.insert(std::map<type,std::string>::value_type(5,"Expired"));
      valueByName.insert(std::map<std::string,type>::value_type("Expired",5));
      nameByValue.insert(std::map<type,std::string>::value_type(7,"Killed"));
      valueByName.insert(std::map<std::string,type>::value_type("Killed",7));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
