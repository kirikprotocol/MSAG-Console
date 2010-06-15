#ifndef __GENERATED_ENUM_CONFIGTYPE__
#define __GENERATED_ENUM_CONFIGTYPE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class ConfigType{
public:
  typedef int8_t type;
  static const type MainConfig;
  static const type Profiles;
  static const type Msc;
  static const type Routes;
  static const type Sme;
  static const type ClosedGroups;
  static const type Aliases;
  static const type MapLimits;
  static const type Regions;
  static const type Resources;
  static const type Reschedule;
  static const type Snmp;
  static const type TimeZones;
  static const type Fraud;
  static const type Acl;
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
      nameByValue.insert(std::map<type,std::string>::value_type(1,"MainConfig"));
      valueByName.insert(std::map<std::string,type>::value_type("MainConfig",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"Profiles"));
      valueByName.insert(std::map<std::string,type>::value_type("Profiles",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"Msc"));
      valueByName.insert(std::map<std::string,type>::value_type("Msc",3));
      nameByValue.insert(std::map<type,std::string>::value_type(4,"Routes"));
      valueByName.insert(std::map<std::string,type>::value_type("Routes",4));
      nameByValue.insert(std::map<type,std::string>::value_type(5,"Sme"));
      valueByName.insert(std::map<std::string,type>::value_type("Sme",5));
      nameByValue.insert(std::map<type,std::string>::value_type(6,"ClosedGroups"));
      valueByName.insert(std::map<std::string,type>::value_type("ClosedGroups",6));
      nameByValue.insert(std::map<type,std::string>::value_type(7,"Aliases"));
      valueByName.insert(std::map<std::string,type>::value_type("Aliases",7));
      nameByValue.insert(std::map<type,std::string>::value_type(8,"MapLimits"));
      valueByName.insert(std::map<std::string,type>::value_type("MapLimits",8));
      nameByValue.insert(std::map<type,std::string>::value_type(9,"Regions"));
      valueByName.insert(std::map<std::string,type>::value_type("Regions",9));
      nameByValue.insert(std::map<type,std::string>::value_type(10,"Resources"));
      valueByName.insert(std::map<std::string,type>::value_type("Resources",10));
      nameByValue.insert(std::map<type,std::string>::value_type(11,"Reschedule"));
      valueByName.insert(std::map<std::string,type>::value_type("Reschedule",11));
      nameByValue.insert(std::map<type,std::string>::value_type(12,"Snmp"));
      valueByName.insert(std::map<std::string,type>::value_type("Snmp",12));
      nameByValue.insert(std::map<type,std::string>::value_type(13,"TimeZones"));
      valueByName.insert(std::map<std::string,type>::value_type("TimeZones",13));
      nameByValue.insert(std::map<type,std::string>::value_type(14,"Fraud"));
      valueByName.insert(std::map<std::string,type>::value_type("Fraud",14));
      nameByValue.insert(std::map<type,std::string>::value_type(15,"Acl"));
      valueByName.insert(std::map<std::string,type>::value_type("Acl",15));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
