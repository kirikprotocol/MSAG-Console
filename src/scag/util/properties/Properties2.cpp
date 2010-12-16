#include <stdlib.h>
#include "Properties2.h"
#include "scag/exc/SCAGExceptions.h"
// #include <memory.h>
// #include "util/recoder/recode_dll.h"
// #include <core/buffers/TmpBuf.hpp>
// #include <iostream>
// using smsc::core::buffers::TmpBuf;

namespace scag2 {
namespace util {
namespace properties {

using namespace scag::exceptions;

const char* TimeFormat = "%A, %d - %B - %Y.\n";

Property::Property() :
sync(false), type(pt_str), i_val(0) { show("ctor"); }

Property::Property( int64_t v ) :
sync(false), type(pt_int), i_val(v) { show("ctor"); }

Property::Property( const string_type& v ) :
sync(false), type(pt_str), i_val(0), s_val(v) { show("ctor"); }

Property::Property( const char* v ) :
sync(false), type(pt_str), i_val(0), s_val(v) { show("ctor"); }


namespace {
smsc::logger::Logger* log_ = 0;
}

void Property::show( const char* where ) const
{
    if (!log_) log_ = smsc::logger::Logger::getInstance("prop");
    const char* stype;
    switch (type) {
    case pt_int: stype="int"; break;
    case pt_str: stype="str"; break;
    case pt_bool: stype="bool"; break;
    case pt_date: stype="date"; break;
    default: stype="???"; break;
    }
    smsc_log_debug(log_,"%s@%p sync=%d type=%s i=%lld s='%s'",
                   where, this, sync, stype, i_val, s_val.empty() ? "" : s_val.c_str() );
}


int64_t Property::convertToInt() const
{
    if (sync) return i_val;

    switch (type) 
    {
    case pt_bool:
    case pt_int:
    case pt_date:
        throw SCAGException("convertToInt: unsupported type");
        break;
    case pt_str:
        i_val = atoi(s_val.c_str());
        break;
    }
    sync = true;
    show("cvtToInt");
    return i_val;
}

const Property::string_type& Property::convertToStr() const
{
    if (sync) return s_val;

    char buff[64];

    switch (type) 
    {
    case pt_str:
        throw SCAGException("convertToStr: unsupported type");
        break;
    case pt_date: {
        const time_t tmp = time_t(i_val);
        tm tmb;
        if ( ! localtime_r(&tmp,&tmb) ) {
            throw SCAGException("localtime_r");
        }
        strftime(buff,sizeof(buff),TimeFormat,&tmb);
        s_val.assign(buff);
        break;
    }
    case pt_bool: 
    case pt_int:
        sprintf(buff, "%lld", i_val);
        s_val.assign(buff);
        break;
    }
    sync = true;
    show("cvtToStr");
    return s_val;
}

/*
bool Property::convertToBool() const
{
    if (sync) return bool(i_val);
    switch (type) 
    {
    case pt_bool:
    case pt_int:
        break;
    case pt_str:
        i_val = s_val.length();
        break;
    }
    sync = true;
    show("cvtToBool");
    return bool(i_val);
}
 */

time_t Property::convertToDate() const
{
    if (sync) return (time_t)i_val;
    time_t val;

    switch (type) 
    {
    case pt_int:
    case pt_bool:
    case pt_date:
        throw SCAGException("convertToDate: unsupported type");
        break;
    case pt_str: 
        tm time;
        strptime(s_val.c_str(),TimeFormat,&time);
        i_val = mktime(&time);
        break;
    }
    sync = true;

    val = (time_t) i_val;
    show("cvtToDate");
    return val;
}

//////////////////////////////////////////////////////SET////////////////////////////////

void Property::setInt(int64_t val) 
{
    // if (constant) throw ConstantSetException();
    sync = false;
    i_val = val;
    type = pt_int;
    show("setInt");
}

void Property::setStr(const string_type& val)
{
    // if (constant) throw ConstantSetException();
    sync = false;
    s_val = val;
    type = pt_str;
    show("setStr");
}

void Property::setBool( bool val )
{
    // if (constant) throw ConstantSetException();
    sync = false; 
    i_val = val;
    type = pt_bool;
    show("setBool");
}

void Property::setDate(time_t val) 
{
    // if (constant) throw ConstantSetException();
    sync = false; 
    i_val = val;
    type = pt_date;
    show("setDate");
}


/////////////////////////////////////////////////COMPARE///////////////////////////////////////

int Property::Compare(const string_type& val) const
{
    return getStr().compare(val);
}

int Property::Compare(bool val) const
{
    return (getBool()!=val);
}

int Property::Compare(int64_t val) const
{
    register int64_t prop = getInt();
    if (prop>val) return 1;
    if (prop<val) return -1;
    return 0;
}

int Property::Compare(const Property& val, bool reqcast) const
{
    if (!reqcast) 
        if (val.type!=type) throw CompareException("Property: cannot compare incomparable properties");
    return Compare(val, type);
}

int Property::Compare(const Property& val, PropertyType pt) const
{
    if (pt == pt_int) 
    {
        return Compare( val.getInt() );
    }
    else if (pt == pt_bool) 
    {
        return Compare( val.getBool() );
    }
    else {
        // string and date types
        return Compare( val.getStr() );
    }
}


// NamedProperty wraps Property's set methods to notify patron (if defined)

void AdapterProperty::setStr(const string_type& val)
{
    Property::setStr(val);
    if (patron) patron->changed(*this);
}
void AdapterProperty::setInt(int64_t val)
{
    Property::setInt(val);
    if (patron) patron->changed(*this);
}
void AdapterProperty::setBool(bool val)
{
    Property::setBool(val);
    if (patron) patron->changed(*this);
}
void AdapterProperty::setDate(time_t val)
{
    Property::setDate(val);
    if (patron) patron->changed(*this);
}

}
}
}
