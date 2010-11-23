#ifndef __GENERATED_ENUM_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_REPORTOPTIONS__
#define __GENERATED_ENUM_SMSC_CLUSTER_CONTROLLER_PROTOCOL_MESSAGES_REPORTOPTIONS__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class ReportOptions{
public:
  typedef int8_t type;
 
  ReportOptions()
  {
    isNull=true;
  }
  ReportOptions(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("ReportOptions",value);
    }
    isNull=false;
  }
 
  ReportOptions(const ReportOptions& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("ReportOptions",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const ReportOptions& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type ReportNone=0;
  static const type ReportFull=1;
  static const type ReportFinal=2;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const ReportOptions& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"ReportNone"));
      valueByName.insert(std::map<std::string,type>::value_type("ReportNone",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"ReportFull"));
      valueByName.insert(std::map<std::string,type>::value_type("ReportFull",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"ReportFinal"));
      valueByName.insert(std::map<std::string,type>::value_type("ReportFinal",2));
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
