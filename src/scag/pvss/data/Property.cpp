/* $Id$ */

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "Property.h"
#include "informer/io/TextEscaper.h"

namespace {

using namespace scag2::pvss;
using namespace scag2::pvss::perstypes;
using namespace scag2::exceptions;

const size_t STRBUF_SIZE = 64;
const char* BOOL_TRUE = "BOOL: true";
const char* BOOL_FALSE = "BOOL: false";
const char* INT_PREF = "INT: ";
const char* STRING_PREF = "STRING: \"";
const char* DATE_PREF = "DATE: ";
const char* TIMEFORMAT = "%Y/%m/%d %H:%M:%S";
const char* FINALDATE = " FINAL_DATE: %Y/%m/%d %H:%M:%S";
const char* FINALDATENONE = " FINAL_DATE: -";
const char* LIFETIME = " LIFE_TIME: ";
#define TIMEPOLPREF " TIME_POLICY: "

size_t timePolicyFromString( const std::string& from, TimePolicy& tp, size_t* start = 0 )
{
    size_t startpos = from.find(TIMEPOLPREF);
    if ( start ) *start = startpos;
    if ( startpos == std::string::npos ) return startpos;
    size_t tpos = startpos + strlen(TIMEPOLPREF);
#define TIMEPOLCMP(x) ( from.substr(tpos,strlen(#x)) == #x ) { tp = x; return tpos + strlen(#x); }
    if TIMEPOLCMP(INFINIT)
        else if TIMEPOLCMP(FIXED)
            else if TIMEPOLCMP(ACCESS)
                else if TIMEPOLCMP(R_ACCESS)
                    else if TIMEPOLCMP(W_ACCESS)
                        else { tp = UNKNOWN; return std::string::npos; }
#undef TIMEPOLCMP
}


smsc::core::synchronization::Mutex logMtx;

} // namespace



namespace scag2 {
namespace pvss {

namespace perstypes {

const char* timePolicyToString( TimePolicy tp )
{
#define TIMEPOLTOSTR(x) case (x) : return TIMEPOLPREF #x
    switch (tp) {
        TIMEPOLTOSTR(UNKNOWN);
        TIMEPOLTOSTR(INFINIT);
        TIMEPOLTOSTR(FIXED);
        TIMEPOLTOSTR(ACCESS);
        TIMEPOLTOSTR(R_ACCESS);
        TIMEPOLTOSTR(W_ACCESS);
#undef TIMEPOLTOSTR
    default: return TIMEPOLPREF "???";
    }
}

#undef TIMEPOLPREF

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


const std::string Property::emptyString_("empty");

smsc::logger::Logger* Property::log_ = 0;

void Property::initLog()
{
    if (!log_) {
        MutexGuard mg(logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("prop.alloc");
    }
}

void Property::setPropertyName(const char* nm) {
  name.clear();
  name.append(nm);
    if ( propertyStr_.empty() ) return;
    propertyStr_.erase(0,name.size()+2);
    propertyStr_.insert(0, std::string("\"") + name + "\"");
}

void Property::assign(const char *nm, const char *str, TimePolicy policy, time_t fd, uint32_t lt)
{
    setValue(str); // will invalidate cache
    setTimePolicy(policy, fd, lt);
    setPropertyName(nm);
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
    if ( &cp == this ) return;
    invalidateCache();
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
    if ( &cp == this ) return;
    setValue(cp);
    setPropertyName(cp.name.c_str());
    time_policy = cp.time_policy;
    final_date = cp.final_date;
    life_time = cp.life_time;
}

const std::string& Property::toString() const
{
    if ( ! propertyStr_.empty() ) return propertyStr_;
    if ( name.empty() ) return emptyString_;
    propertyStr_.reserve( name.size() + 40 );
    char strBuf[STRBUF_SIZE];
    propertyStr_.push_back('"');
    propertyStr_.append(name);
    propertyStr_.append("\" ");
    switch(type)
    {
    case INT:
        snprintf(strBuf, STRBUF_SIZE, "%s%lld", INT_PREF, static_cast<long long int>(i_val));
        propertyStr_.append(strBuf);
        break;
    case STRING: {
        propertyStr_.append(STRING_PREF);
        // escaping the property
        eyeline::informer::TmpBuf<char,512> propbuf;
        eyeline::informer::escapeText(propbuf,s_val.c_str(),strlen(s_val.c_str()),true);
        propertyStr_.append(propbuf.get());
        propertyStr_.push_back('"');
        break;
    }
    case BOOL:
        propertyStr_.append(b_val ? BOOL_TRUE : BOOL_FALSE);
        break;
    case DATE:
        propertyStr_.append(DATE_PREF);
        struct tm res;
        if ( 0 == strftime(strBuf, STRBUF_SIZE, TIMEFORMAT, gmtime_r(&d_val,&res)) ) {
            // failure
            snprintf(strBuf,STRBUF_SIZE,"not fit");
        }
        propertyStr_.append(strBuf);
        break;
    }

    // propertyStr_.append(" ");
    propertyStr_.append(timePolicyToString(time_policy));

    if(time_policy != INFINIT)
    {
        struct tm res;
        strftime(strBuf, STRBUF_SIZE, FINALDATE, gmtime_r(&final_date,&res));
        propertyStr_.append(strBuf);
        snprintf(strBuf, STRBUF_SIZE, "%s%d", LIFETIME, life_time);
        propertyStr_.append(strBuf);
    }
    return propertyStr_;
}


void Property::fromString( const std::string& input ) /* throw (exceptions::IOException) */ 
{
    // cleanup
    invalidateCache();
    type = PropertyType(0);
    i_val = 0;
    s_val.clear();
    time_policy = INFINIT;

    if ( input.empty() ) throw exceptions::IOException("property string is empty");
    if ( input == "empty" ) return;
    if ( input[0] != '"' ) throw exceptions::IOException("property string does not have a name");

    std::string from;
    // parsing name
    std::string newName;
    {
        size_t quote = input.find("\"",1);
        if ( quote == std::string::npos ) throw exceptions::IOException("closing quote is not found");
        newName = input.substr(1,quote-1);
        size_t notblank = input.find_first_not_of(" ",quote+1);
        if ( notblank == std::string::npos ) throw exceptions::IOException("property tail not found");
        from = input.substr(notblank);
        setPropertyName(newName.c_str());
    }
        
    // parsing type and value
    if ( from.substr(0,strlen(INT_PREF)) == INT_PREF ) {
        // int
        from.erase(0,strlen(INT_PREF));
        char* endptr;
        type = INT;
        i_val = strtoll(from.c_str(),&endptr,10);
        from.erase(0,endptr-from.c_str());
    } else if ( from.substr(0,strlen(STRING_PREF)) == STRING_PREF ) {
        // string
        from.erase(0,strlen(STRING_PREF));
        size_t quote = 0;
        do {
            quote = from.find(quote,'"');
            if ( quote == std::string::npos ) throw exceptions::IOException("closing quote on string value is not found");
            if ( quote+1 >= from.size() ) throw exceptions::IOException("closing quote is the last symbol in the string");
            if ( from[quote+1] == '"' ) {
                // there is another quote, treat them as a single one
                quote += 2;
                continue;
            }
            break;
        } while (true);
        type = STRING;
        eyeline::informer::TmpBuf<char,512> propbuf;
        propbuf.reserve(quote+1);
        s_val = eyeline::informer::unescapeText(propbuf.get(),from.c_str(),
                                                quote,true);
        // s_val = from.substr(0,quote);
        from.erase(0,quote+1);
    } else if ( from.substr(0,strlen(BOOL_TRUE)) == BOOL_TRUE ) {
        // bool: true
        type = BOOL;
        b_val = true;
        from.erase(0,strlen(BOOL_TRUE));
    } else if ( from.substr(0,strlen(BOOL_FALSE)) == BOOL_FALSE ) {
        // bool: false
        type = BOOL;
        b_val = false;
        from.erase(0,strlen(BOOL_FALSE));
    } else if ( from.substr(0,strlen(DATE_PREF)) == DATE_PREF ) {
        // date
        from.erase(0,strlen(DATE_PREF));
        struct tm indate;
        indate.tm_isdst = 0;
        char* p = strptime(from.c_str(),TIMEFORMAT,&indate);
        if ( !p ) throw exceptions::IOException("cannot convert time value");
        time_t ourdate = mktime(&indate) - timezone;
        if ( ourdate == time_t(-1) ) throw exceptions::IOException("cannot convert to time_t");
        type = DATE;
        d_val = ourdate;
        from.erase(0,p - from.c_str());
    } else {
        throw exceptions::IOException("unknown property type/value: %s", from.c_str() );
    }

    // parse time policy
    {
        TimePolicy tp;
        size_t startpos;
        size_t next = timePolicyFromString( from, tp, &startpos );
        if ( next == std::string::npos ) throw exceptions::IOException("cannot read time policy type: %s", from.c_str());
        time_policy = tp;
        size_t lastquote;
        if ( type == STRING && startpos > 1 && (lastquote = from.rfind("\"",startpos)) != std::string::npos ) {
            // string value contains a quote
            s_val.reserve( s_val.size() + 2 + lastquote );
            s_val.push_back('"');
            s_val.append(from.substr(0,lastquote));
        }
        from.erase(0,next);
    }

    // read final date and life time
    if ( time_policy != INFINIT ) {
        if (strncmp(from.c_str(),FINALDATENONE,strlen(FINALDATENONE))==0) {
            from.erase(0,strlen(FINALDATENONE));
            final_date = time_t(-1);
        } else {
            // final date
            struct tm indate;
            indate.tm_isdst = 0;
            char* pdate = strptime(from.c_str(),FINALDATE,&indate);
            if ( !pdate ) throw exceptions::IOException("cannot convert final date");
            time_t ourdate = mktime(&indate) - timezone;
            if ( ourdate == time_t(-1)) throw exceptions::IOException("wrong final date");
            from.erase(0,pdate-from.c_str());
            final_date = ourdate;
        }
        {
            // life time
            if ( from.substr(0,strlen(LIFETIME)) != LIFETIME )
                throw exceptions::IOException("cannot convert life time");
            from.erase(0,strlen(LIFETIME));
            char* endptr;
            uint32_t lt = strtoul(from.c_str(),&endptr,10);
            from.erase(0,endptr-from.c_str());
            life_time = lt;
            setTimePolicy(time_policy,final_date,life_time);
        }
    } else {
        final_date = 0;
        life_time = 0;
    }
}


Property::Property(const Property& cp)
{
    copy(cp);
    // smsc_log_debug(log_,"ctor %p %s",this,toString().c_str());
}

Property& Property::operator=(const Property& cp)
{
    copy(cp);
    return *this;
}

void Property::setTimePolicy(TimePolicy policy, time_t fd, uint32_t lt)
{
    invalidateCache();
    time_policy = policy;
    life_time = lt;
    final_date = (policy == FIXED && fd != -1) ? fd : time(NULL) + lt;
}

void Property::ReadAccess()
{
    if (time_policy == R_ACCESS || time_policy == ACCESS) {
        invalidateCache();
        final_date = time(NULL) + life_time;
    }
}

void Property::WriteAccess()
{
    if (time_policy == W_ACCESS || time_policy == ACCESS) {
        invalidateCache();
        final_date = time(NULL) + life_time;
    }
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
    setIntValue(i);
    setPropertyName(nm);
    setTimePolicy(policy, fd, lt);
}

void Property::setBool(const char *nm, bool b, TimePolicy policy, time_t fd, uint32_t lt)
{
    setBoolValue(b);
    setPropertyName(nm);
    setTimePolicy(policy, fd, lt);
}

void Property::setString(const char *nm, const char* str, TimePolicy policy, time_t fd, uint32_t lt)
{
    setStringValue(str);
    setPropertyName(nm);
    setTimePolicy(policy, fd, lt);
}

void Property::setDate(const char *nm, time_t t, TimePolicy policy, time_t fd, uint32_t lt)
{
    setDateValue(t);
    setPropertyName(nm);
    setTimePolicy(policy, fd, lt);
}

void Property::Serialize( util::storage::SerialBuffer& buf, bool toFSDB, util::io::GlossaryBase* glossary) const
{
    buf.WriteInt8((uint8_t)type);
    buf.WriteInt8((uint8_t)time_policy);
    buf.WriteInt32((uint32_t)final_date);
    buf.WriteInt32(life_time);
    if(toFSDB) {
        int i_name;
        if(util::io::GlossaryBase::NO_VALUE == (i_name = glossary->GetValueByKey(name))) {
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

void Property::Deserialize(util::storage::SerialBuffer& buf, bool fromFSDB, util::io::GlossaryBase* glossary)
{
    invalidateCache();
    type = (PropertyType)buf.ReadInt8();
    time_policy = (TimePolicy)buf.ReadInt8();
    final_date = (time_t)buf.ReadInt32();
    life_time = buf.ReadInt32();
    if (fromFSDB) {
        int i_name = buf.ReadInt32();
        if(util::io::GlossaryBase::SUCCESS != glossary->GetKeyByValue(i_name, name)) {
            char buff[32];
            snprintf(buff, 32, "%d", i_name);
            setPropertyName(buff);
        }
    } else {
        buf.ReadString(name);
    }

    switch(type) {
        case INT:   i_val = static_cast<int32_t>(buf.ReadInt32()); break;
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
