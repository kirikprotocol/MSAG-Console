/* $Id$ */

#include <string.h>
#include <stdlib.h>
#include "Property.h"

namespace scag{ namespace pers{

void Property::assign(const char *nm, const char *str, TimePolicy policy, time_t fd, uint32_t lt)
{
    name = nm;
    setTimePolicy(policy, fd, lt);
    setValue(str);
}

void Property::setValue(const char* str) {
  if (!str) {
    setStringValue("");
    return;
  }
  char *ep;
  int32_t i = 0;
  if(!strcmp(str, "false"))
      setBoolValue(false);
  else if(!strcmp(str, "true"))
      setBoolValue(true);
  else if((!(i = strtol(str, &ep, 0)) && (*str != '0' || strlen(str) != 1))
             || i == LONG_MAX || i == LONG_MIN || *ep)
    setStringValue(str);
  else
    setIntValue(i);
  //else if((str[0] >= '0' && str[0] <= '9' || str[0] == '-' || str[0] == '+') 
    //  && ((i = strtol(str, &ep, 0)) || ep != str) && !*ep)
     // setIntValue(i);
//    else if((s = strptime(str, "%d.%m.%Y %T", &time)) && !*s)
//        setDateValue(mktime(&time));
  //else
      //setStringValue(str);
}

void Property::setValue(const Property &cp)
{
    type = cp.type;

    switch(type) {
        case INT:
            i_val = cp.i_val;
            break;
        case STRING:
            s_val = cp.s_val;
            break;
        case BOOL:
            b_val = cp.b_val;
            break;
        case DATE:
            d_val = cp.d_val;
            break;
    }
}

void Property::copy(const Property& cp)
{
    name = cp.name;
    time_policy = cp.time_policy;
    final_date = cp.final_date;
    life_time = cp.life_time;

    setValue(cp);
}

std::string Property::toString() const
{
    char buf[32];
    std::string str;

    str = '"' + name + '"';
    switch(type)
    {
        case INT:
            sprintf(buf, " INT: %d", i_val);
            str += buf;
            break;
        case STRING:
            str += " STRING: \"" + s_val + '"';
            break;
        case BOOL:
            str += " BOOL: ";
            str += (b_val ? "true" : "false");
            break;
        case DATE:
            strftime(buf, 32, "%Y/%m/%d %H:%M:%S", gmtime(&d_val));
            str += " DATE: ";
            str += buf;
            break;
    }

    str += " TIME_POLICY: ";
    switch(time_policy)
    {
        case FIXED:
            str += "FIXED";
            break;
        case INFINIT:
            str += "INFINIT";
            break;
        case R_ACCESS:
            str += "R_ACCESS";
            break;
        case W_ACCESS:
            str += "W_ACCESS";
            break;
        case ACCESS:
            str += "ACCESS";
            break;
    }
    if(time_policy != INFINIT)
    {
        strftime(buf, 32, "%Y/%m/%d %H:%M:%S", gmtime(&final_date));
        str += " FINAL_DATE: ";
        str += buf;
        sprintf(buf, "%d", life_time);
        str += " LIFE_TIME: ";
        str += buf;
    }
    return str;
}

Property::Property(const Property& cp)
{
    copy(cp);
}

Property& Property::operator=(const Property& cp)
{
    copy(cp);
    return *this;
}

void Property::setTimePolicy(TimePolicy policy, time_t fd, uint32_t lt)
{
    time_policy = policy;
    life_time = lt;
    final_date = (policy == FIXED && fd != -1) ? fd : final_date = time(NULL) + lt;
}

void Property::ReadAccess()
{
    if(time_policy == R_ACCESS || time_policy == ACCESS)
        final_date = time(NULL) + life_time;
}

void Property::WriteAccess()
{
    if(time_policy == W_ACCESS || time_policy == ACCESS)
        final_date = time(NULL) + life_time;
}

bool Property::isExpired()
{
    return time_policy != INFINIT && final_date <= time(NULL);
}
bool Property::isExpired(time_t cur_time)
{
	return time_policy != INFINIT && final_date <= cur_time;
}
void Property::setInt(const char *nm, int32_t i, TimePolicy policy, time_t fd, uint32_t lt)
{
    name = nm;
    setIntValue(i);
    setTimePolicy(policy, fd, lt);
}

void Property::setBool(const char *nm, bool b, TimePolicy policy, time_t fd, uint32_t lt)
{
    name = nm;
    setBoolValue(b);
    setTimePolicy(policy, fd, lt);
}

void Property::setString(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt)
{
    name = nm;
    setStringValue(str);
    setTimePolicy(policy, fd, lt);
}

void Property::setDate(const char *nm, time_t t, TimePolicy policy, time_t fd, uint32_t lt)
{
    name = nm;
    setDateValue(t);
    setTimePolicy(policy, fd, lt);
}

void Property::Serialize(SerialBuffer& buf, bool toFSDB)
{
    buf.WriteInt8((uint8_t)type);
    buf.WriteInt8((uint8_t)time_policy);
    buf.WriteInt32((uint32_t)final_date);
    buf.WriteInt32(life_time);
    if(toFSDB)
    {
	int i_name;
	if(Glossary::NO_VALUE == (i_name = Glossary::GetValueByKey(name)))
		i_name = Glossary::Add(name);
    	buf.WriteInt32(i_name);
    }
    else
	buf.WriteString(name.c_str());

    switch(type) {
        case INT:   buf.WriteInt32(i_val);          break;
        case STRING:buf.WriteString(s_val.c_str()); break;
        case BOOL:  buf.WriteInt8((uint8_t)b_val);  break;
        case DATE:  buf.WriteInt32((uint32_t)d_val);break;
    }
}

void Property::Deserialize(SerialBuffer& buf, bool fromFSDB)
{
    type = (PropertyType)buf.ReadInt8();
    time_policy = (TimePolicy)buf.ReadInt8();
    final_date = (time_t)buf.ReadInt32();
    life_time = buf.ReadInt32();
    if(fromFSDB)
    {
	int i_name = buf.ReadInt32();
	if(Glossary::SUCCESS != Glossary::GetKeyByValue(i_name, name))
	{
		char buff[32];
		snprintf(buff, 32, "%d", i_name);
		name = buff;
	}
    }
    else
	buf.ReadString(name);

    switch(type) {
        case INT:   i_val = buf.ReadInt32();        break;
        case STRING:buf.ReadString(s_val);          break;
        case BOOL:  b_val = (bool)buf.ReadInt8();   break;
        case DATE:  d_val = (time_t)buf.ReadInt32();break;
    }
}

bool Property::convertToInt() {
  switch (type) {
  case STRING: {
    char *ep;
    int32_t conv_val = strtol(s_val.c_str(), &ep, 0);
    if((!conv_val && (s_val[0] != '0' || s_val.size() != 1)) 
       || conv_val == LONG_MAX || conv_val == LONG_MIN || *ep) {
      return false;
    }
    setIntValue(conv_val);
    return true;
  }
  case INT: return true;
  case BOOL: setIntValue((int32_t)b_val); return true;
  case DATE: return false;
  }
  return  false;
}

}}
