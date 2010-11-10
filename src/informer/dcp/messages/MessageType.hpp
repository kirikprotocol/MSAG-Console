#ifndef __GENERATED_ENUM_EYELINE_INFORMER_DCP_MESSAGES_MESSAGETYPE__
#define __GENERATED_ENUM_EYELINE_INFORMER_DCP_MESSAGES_MESSAGETYPE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{


class MessageType{
public:
  typedef int8_t type;
 
  MessageType()
  {
    isNull=true;
  }
  MessageType(const type& argValue):value(argValue)
  {
    if(!isValidValue(value))
    {
      throw eyeline::protogen::framework::InvalidEnumValue("MessageType",value);
    }
    isNull=false;
  }
 
  MessageType(const MessageType& argRhs):value(argRhs.value),isNull(argRhs.isNull)
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
      throw eyeline::protogen::framework::InvalidEnumValue("MessageType",argValue);
    }
    value=argValue;
    isNull=false;
  }
 
  bool operator==(const MessageType& rhs)const
  {
    return isNull==rhs.isNull && value==rhs.value;
  }
 
  static const type TextMessage=1;
  static const type GlossaryMessage=2;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
  }

  static std::string getNameByValue(const MessageType& value)
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"TextMessage"));
      valueByName.insert(std::map<std::string,type>::value_type("TextMessage",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"GlossaryMessage"));
      valueByName.insert(std::map<std::string,type>::value_type("GlossaryMessage",2));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
