#ifndef _PROP_OBJECT_H
#define _PROP_OBJECT_H

#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>
#include <sms/sms.h>

using namespace smsc::sms;
using namespace smsc::sms::Tag;


class Prop
{

 public:

 void setStrProp(int tag,std::string pr)
 {
	sms.setStrProperty(tag,pr.c_str());
 };

 void setIntProp(int tag,uint32_t pr)
 { 
        sms.setIntProperty(tag,pr);
 };

 const string& getStrProp(int tag)
 {
        return sms.getStrProperty(tag);
 };

 uint32_t  getIntProp(uint32_t tag)
 { 
    return sms.getIntProperty(tag);
 };

 const SMS& getSMS()
 {
   return sms;
 };
 
 private:

 SMS sms;

};

#endif