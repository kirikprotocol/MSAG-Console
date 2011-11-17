#ifndef __GENERATED_ENUM_SMSC_MCISME_ADMIN_MESSAGES_STATUS__
#define __GENERATED_ENUM_SMSC_MCISME_ADMIN_MESSAGES_STATUS__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace smsc{
namespace mcisme{
namespace admin{
namespace messages{


class Status{
public:
  typedef int8_t type;
 
  Status()
  {
    isNull=true;
  }
  Status(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("Status",value);
    }
    isNull=false;
  }
 
  Status(const Status& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("Status",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const Status& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type OK=0;
  static const type SystemError=1;
  static const type SubscriberNotFound=2;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const Status& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"OK"));
      valueByName.insert(std::map<std::string,type>::value_type("OK",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"SystemError"));
      valueByName.insert(std::map<std::string,type>::value_type("SystemError",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"SubscriberNotFound"));
      valueByName.insert(std::map<std::string,type>::value_type("SubscriberNotFound",2));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
