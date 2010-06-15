#ifndef __GENERATED_ENUM_BILLINGMODE__
#define __GENERATED_ENUM_BILLINGMODE__

#include <map>
#include <string>
#include <inttypes.h>
#include "eyeline/protogen/framework/Exceptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{


class BillingMode{
public:
  typedef int8_t type;
  static const type BillOff;
  static const type BillOn;
  static const type BillMt;
  static const type BillFr;
  static const type BillOnSubmit;
  static const type BillCdr;
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
      nameByValue.insert(std::map<type,std::string>::value_type(0,"BillOff"));
      valueByName.insert(std::map<std::string,type>::value_type("BillOff",0));
      nameByValue.insert(std::map<type,std::string>::value_type(1,"BillOn"));
      valueByName.insert(std::map<std::string,type>::value_type("BillOn",1));
      nameByValue.insert(std::map<type,std::string>::value_type(2,"BillMt"));
      valueByName.insert(std::map<std::string,type>::value_type("BillMt",2));
      nameByValue.insert(std::map<type,std::string>::value_type(3,"BillFr"));
      valueByName.insert(std::map<std::string,type>::value_type("BillFr",3));
      nameByValue.insert(std::map<type,std::string>::value_type(4,"BillOnSubmit"));
      valueByName.insert(std::map<std::string,type>::value_type("BillOnSubmit",4));
      nameByValue.insert(std::map<type,std::string>::value_type(5,"BillCdr"));
      valueByName.insert(std::map<std::string,type>::value_type("BillCdr",5));
    }
  };
  static StaticInitializer staticInitializer;
};

}
}
}
}


#endif
