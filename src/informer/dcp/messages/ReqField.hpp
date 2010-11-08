#ifndef __GENERATED_ENUM_REQFIELD__
#define __GENERATED_ENUM_REQFIELD__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class ReqField{
public:
  typedef int8_t type;
 
  ReqField()
  {
    isNull=true;
  }
  ReqField(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("ReqField",value);
    }
    isNull=false;
  }
 
  ReqField(const ReqField& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("ReqField",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const ReqField& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type State=1;
  static const type Date=2;
  static const type Abonent=3;
  static const type Text=4;
  static const type ErrorCode=5;
  static const type UserData=6;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const ReqField& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"State"));
      valueByName.insert(std::map<std::string,type>::value_type("State",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"Date"));
      valueByName.insert(std::map<std::string,type>::value_type("Date",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"Abonent"));
      valueByName.insert(std::map<std::string,type>::value_type("Abonent",3));
      nameByValue.insert(std::map<type,std::string>::value_type(4,"Text"));
      valueByName.insert(std::map<std::string,type>::value_type("Text",4));
      nameByValue.insert(std::map<type,std::string>::value_type(5,"ErrorCode"));
      valueByName.insert(std::map<std::string,type>::value_type("ErrorCode",5));
      nameByValue.insert(std::map<type,std::string>::value_type(6,"UserData"));
      valueByName.insert(std::map<std::string,type>::value_type("UserData",6));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
