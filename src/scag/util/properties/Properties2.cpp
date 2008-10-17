#include "Properties2.h"
#include "scag/exc/SCAGExceptions.h"
// #include <cstdlib.h>
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

int64_t Property::convertToInt() const
{
    if (sync) return i_val;

    switch (type) 
    {
    case pt_bool:
    case pt_int:
    case pt_date:
        break;
    case pt_str:
        i_val = atoi(s_val.c_str());
        break;
        /*
    case pt_date:
        throw ConvertException("date","int");
        break;
         */
    }
    sync = true;
    return i_val;
}

const Property::string_type& Property::convertToStr() const
{
    if (sync) return s_val;

    char buff[128];

    switch (type) 
    {
    case pt_str:
        break;
    case pt_bool: 
    case pt_int:
        sprintf(buff, "%lld", i_val);
        s_val.assign(buff);
        break;
    }
    sync = true;
    return s_val;
}

bool Property::convertToBool() const
{
    if (sync) return (bool)i_val;
    bool val;
    // char buff[1024];
    switch (type) 
    {
    case pt_bool:
    case pt_int:
        break;
    case pt_str:
        i_val = s_val.length();
        break;
    }
    val = i_val;
    sync = true;
    return val;
}

time_t Property::convertToDate() const
{
    if (sync) return (time_t)i_val;
    time_t val;

    switch (type) 
    {
    case pt_str: 
        tm time;

        strptime(s_val.c_str(),TimeFormat,&time);
        i_val = mktime(&time);
        break;
    }
    sync = true;

    val = (time_t) i_val;
    return val;
}

//////////////////////////////////////////////////////SET////////////////////////////////

void Property::setInt(int64_t val) 
{
    // if (constant) throw ConstantSetException();
    sync = false;
    i_val = val;
    type = pt_int;
}

void Property::setStr(const string_type& val)
{
    // if (constant) throw ConstantSetException();
    sync = false;
    s_val = val;
    type = pt_str;
}

void Property::setBool( bool val )
{
    // if (constant) throw ConstantSetException();
    sync = false; 
    i_val = val;
    type = pt_bool;
}

void Property::setDate(time_t val) 
{
    // if (constant) throw ConstantSetException();
    sync = false; 
    i_val = val;
    type = pt_date;
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
