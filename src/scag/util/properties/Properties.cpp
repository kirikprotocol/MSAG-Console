
#include <string.h>

#include "Properties.h"

namespace scag { namespace util { namespace properties 
{

const char* ConvertException::message     = "Failed to convert '%s' to type %s";
const char* ConstantSetException::message = "Failed to assign value to constant";

// TODO: Implement get/set methods (check const in set methods!)

int64_t Property::convertToInt() 
{
    if (sync) return i_val;
    int64_t val;
    //  TODO: convert from type to int
    return val;
}
const std::string& Property::convertToStr() 
{
    if (sync) return s_val;
    std::string val;
    // TODO: convert from type to str
    return val;
}
bool Property::convertToBool() 
{
    if (sync) return (bool)i_val;
    bool val;
    // TODO: convert from type to bool
    return val;
}
time_t Property::convertToDate()
{
    if (sync) return (time_t)i_val;
    time_t val;
    // TODO: convert from type to date
    return val;
}

int64_t Property::getInt() 
{
    if (type == PTYPE_INT) return i_val;
    else return convertToInt();
}
const std::string& Property::getString()
{
    if (type == PTYPE_STR) return s_val;
    else return convertToStr();
}
bool Property::getBool()
{
    if (type == PTYPE_BOOL) return (bool)i_val;
    else return convertToBool();
}
time_t Property::getDate() 
{
    if (type == PTYPE_DATE) return (time_t)i_val;
    else return convertToDate();
}

void Property::setInt(int64_t val) 
{
    if (constant) throw ConstantSetException();
    if (type == PTYPE_INT) { sync = false; i_val = val; }
    else {

    }
}
void Property::setString(const std::string& val)
{
    if (constant) throw ConstantSetException();
    if (type == PTYPE_STR) { sync = false; s_val = val; }
    else {

    }
}
void Property::setBool(bool val)
{
    if (constant) throw ConstantSetException();
    if (type == PTYPE_BOOL) { sync = false; i_val = val; }
    else {

    }
}
void Property::setDate(time_t val) 
{
    if (constant) throw ConstantSetException();
    if (type == PTYPE_DATE) { sync = false; i_val = val; }
    else {

    }
}

// NamedProperty wraps Property's set methods to notify patron (if defined)

void NamedProperty::setStr(const std::string& val)
{
    Property::setStr(val);
    if (patron) patron->changed(*this);
}
void NamedProperty::setInt(int64_t val)
{
    Property::setInt(val);
    if (patron) patron->changed(*this);
}
void NamedProperty::setBool(bool val)
{
    Property::setBool(val);
    if (patron) patron->changed(*this);
}
void NamedProperty::setDate(time_t val)
{
    Property::setDate(val);
    if (patron) patron->changed(*this);
}

}}}
