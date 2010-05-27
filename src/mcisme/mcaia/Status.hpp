#ifndef __GENERATED_ENUM_STATUS__
#define __GENERATED_ENUM_STATUS__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace smsc{
namespace mcisme{
namespace mcaia{


class Status{
public:
  typedef int8_t type;
  static const type OK;
  static const type InvalidCalledAddress;
  static const type QueueFull;
  static bool isValidValue(const type& value)
  {
    return nameByValue.find(value)!=nameByValue.end();
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
  static std::map<type,std::string> nameByValue;
  static std::map<std::string,type> valueByName;
  struct StaticInitializer{
    StaticInitializer()
    {
      nameByValue.insert(std::map<type,std::string>::value_type(0,"OK"));
      valueByName.insert(std::map<std::string,type>::value_type("OK",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"InvalidCalledAddress"));
      valueByName.insert(std::map<std::string,type>::value_type("InvalidCalledAddress",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"QueueFull"));
      valueByName.insert(std::map<std::string,type>::value_type("QueueFull",2));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}


#endif
