#ifndef __GENERATED_ENUM_PROFILECHARSET__
#define __GENERATED_ENUM_PROFILECHARSET__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace smsc{
namespace cluster{
namespace controller{
namespace protocol{
namespace messages{


class ProfileCharset{
public:
  typedef int8_t type;
  static const type Default;
  static const type Latin1;
  static const type Ucs2;
  static const type Ucs2AndLatin1;
  static const type UssdIn7BitFlag;
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"Default"));
      valueByName.insert(std::map<std::string,type>::value_type("Default",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"Latin1"));
      valueByName.insert(std::map<std::string,type>::value_type("Latin1",1));
      nameByValue.insert(std::map<type,std::string>::value_type(8,"Ucs2"));
      valueByName.insert(std::map<std::string,type>::value_type("Ucs2",8));
      nameByValue.insert(std::map<type,std::string>::value_type(9,"Ucs2AndLatin1"));
      valueByName.insert(std::map<std::string,type>::value_type("Ucs2AndLatin1",9));
      nameByValue.insert(std::map<type,std::string>::value_type(128,"UssdIn7BitFlag"));
      valueByName.insert(std::map<std::string,type>::value_type("UssdIn7BitFlag",128));
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
