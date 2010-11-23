#ifndef __GENERATED_ENUM_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEBINDMODE__
#define __GENERATED_ENUM_EYELINE_CLUSTERCONTROLLER_PROTOCOL_MESSAGES_SMEBINDMODE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class SmeBindMode{
public:
  typedef int8_t type;
 
  SmeBindMode()
  {
    isNull=true;
  }
  SmeBindMode(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("SmeBindMode",value);
    }
    isNull=false;
  }
 
  SmeBindMode(const SmeBindMode& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("SmeBindMode",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const SmeBindMode& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type modeTx=0;
  static const type modeRx=1;
  static const type modeTrx=2;
  static const type modeUnknown=3;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const SmeBindMode& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"modeTx"));
      valueByName.insert(std::map<std::string,type>::value_type("modeTx",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"modeRx"));
      valueByName.insert(std::map<std::string,type>::value_type("modeRx",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"modeTrx"));
      valueByName.insert(std::map<std::string,type>::value_type("modeTrx",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"modeUnknown"));
      valueByName.insert(std::map<std::string,type>::value_type("modeUnknown",3));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
