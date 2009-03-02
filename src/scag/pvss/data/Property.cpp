/* $Id$ */

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "Property.h"

namespace scag2 {
namespace pvss {

namespace perstypes {

const char* timePolicyToString( TimePolicy tp )
{
#define TIMEPOLTOSTR(x) case (x) : return " TIME_POLICY: " #x
    switch (tp) {
        TIMEPOLTOSTR(UNKNOWN);
        TIMEPOLTOSTR(INFINIT);
        TIMEPOLTOSTR(FIXED);
        TIMEPOLTOSTR(ACCESS);
        TIMEPOLTOSTR(R_ACCESS);
        TIMEPOLTOSTR(W_ACCESS);
#undef TIMEPOLTOSTR
    default: return " TIME_POLICY: ???";
    }
}


const char* propertyTypeToString( PropertyType pt )
{
#define PROPTYPETOSTR(x) case (x) : return #x
    switch (pt) {
        PROPTYPETOSTR(INT);
        PROPTYPETOSTR(STRING);
        PROPTYPETOSTR(BOOL);
        PROPTYPETOSTR(DATE);
#undef PROPTYPETOSTR
    default : return "???";
    }
}

} // namespace perstypes


static const char* BOOL_TRUE = " BOOL: true";
static const char* BOOL_FALSE = " BOOL: false";
static const char* STRING_PREF = " STRING: \"";

void Property::setPropertyName(const char* nm) {
  name.clear();
  name.append(nm);
  propertyStr.clear();
  propertyStr.append(1, '"');
  propertyStr.append(name);
  propertyStr.append(1, '"');
}

void Property::assign(const char *nm, const char *str, TimePolicy policy, time_t fd, uint32_t lt)
{
    setPropertyName(nm);
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
    setPropertyName(cp.name.c_str());
    time_policy = cp.time_policy;
    final_date = cp.final_date;
    life_time = cp.life_time;

    setValue(cp);
}

const std::string& Property::toString() const
{
    memset(strBuf, 0, STRBUF_SIZE);

    propertyStr.erase(name.size() + 2);
    switch(type)
    {
        case INT:
            sprintf(strBuf, " INT: %lld", static_cast<long long int>(i_val));
            propertyStr.append(strBuf);
            break;
        case STRING:
            propertyStr.append(STRING_PREF);
            propertyStr.append(s_val);
            propertyStr += '"';
            break;
        case BOOL:
            propertyStr.append(b_val ? BOOL_TRUE : BOOL_FALSE);
            break;
        case DATE:
            strftime(strBuf, STRBUF_SIZE, " DATE: %Y/%m/%d %H:%M:%S", gmtime(&d_val));
            propertyStr.append(strBuf);
            break;
    }

    propertyStr.append(timePolicyToString(time_policy));

    if(time_policy != INFINIT)
    {
        strftime(strBuf, STRBUF_SIZE, " FINAL_DATE: %Y/%m/%d %H:%M:%S", gmtime(&final_date));
        propertyStr.append(strBuf);
        sprintf(strBuf, " LIFE_TIME: %d", life_time);
        propertyStr.append(strBuf);
    }
    return propertyStr;
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

bool Property::isExpired() const
{
    return time_policy != INFINIT && final_date <= time(NULL);
}
bool Property::isExpired(time_t cur_time) const
{
	return time_policy != INFINIT && final_date <= cur_time;
}
void Property::setInt(const char *nm, int32_t i, TimePolicy policy, time_t fd, uint32_t lt)
{
    setPropertyName(nm);
    setIntValue(i);
    setTimePolicy(policy, fd, lt);
}

void Property::setBool(const char *nm, bool b, TimePolicy policy, time_t fd, uint32_t lt)
{
    setPropertyName(nm);
    setBoolValue(b);
    setTimePolicy(policy, fd, lt);
}

void Property::setString(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt)
{
    setPropertyName(nm);
    setStringValue(str);
    setTimePolicy(policy, fd, lt);
}

void Property::setDate(const char *nm, time_t t, TimePolicy policy, time_t fd, uint32_t lt)
{
    setPropertyName(nm);
    setDateValue(t);
    setTimePolicy(policy, fd, lt);
}

void Property::Serialize( util::storage::SerialBuffer& buf, bool toFSDB, util::storage::GlossaryBase* glossary) const
{
    buf.WriteInt8((uint8_t)type);
    buf.WriteInt8((uint8_t)time_policy);
    buf.WriteInt32((uint32_t)final_date);
    buf.WriteInt32(life_time);
    if(toFSDB) {
        int i_name;
        if(util::storage::GlossaryBase::NO_VALUE == (i_name = glossary->GetValueByKey(name))) {
            i_name = glossary->Add(name);
        }
    	buf.WriteInt32(i_name);
    } else {
        buf.WriteString(name.c_str());
    }

    switch(type) {
        case INT:   buf.WriteInt32(static_cast<uint32_t>(i_val)); break;
        case STRING:buf.WriteString(s_val.c_str());               break;
        case BOOL:  buf.WriteInt8((uint8_t)b_val);                break;
        case DATE:  buf.WriteInt32((uint32_t)d_val);              break;
    }
}

void Property::Deserialize(util::storage::SerialBuffer& buf, bool fromFSDB, util::storage::GlossaryBase* glossary)
{
    type = (PropertyType)buf.ReadInt8();
    time_policy = (TimePolicy)buf.ReadInt8();
    final_date = (time_t)buf.ReadInt32();
    life_time = buf.ReadInt32();
    if (fromFSDB) {
        int i_name = buf.ReadInt32();
        if(util::storage::GlossaryBase::SUCCESS != glossary->GetKeyByValue(i_name, name)) {
            char buff[32];
            snprintf(buff, 32, "%d", i_name);
            setPropertyName(buff);
        }
    } else {
        buf.ReadString(name);
    }

    propertyStr.clear();
    propertyStr.append(1, '"');
    propertyStr.append(name);
    propertyStr.append(1, '"');
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

}//pvss
}//scag
