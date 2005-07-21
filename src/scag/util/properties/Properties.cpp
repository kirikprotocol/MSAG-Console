#include <string.h>

#include "Properties.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>


namespace scag { namespace util { namespace properties 
{

using namespace std;

const char* ConvertException::message     = "Failed to convert '%s' to type %s";
const char* ConstantSetException::message = "Failed to assign value to constant";
const char* TimeFormat = "%A, %d - %B - %Y.\n";

// TODO: Implement get/set methods (check const in set methods!)


//////////////////////////////////////////////////////CONVERT////////////////////////////////



int64_t Property::convertToInt() 
{
    if (sync) return i_val;
    int64_t val;

    switch (type) 
    {
    case pt_bool: 
        break;
    case pt_str:
        i_val = atoi(s_val.c_str());
        break;
/*    case PTYPE_DATE:
        throw ConvertException("date","int");
        break;*/
    }

    val = i_val;
    sync = true;
    return val;
}
const std::string& Property::convertToStr() 
{
    if (sync) return s_val;

    switch (type) 
    {
    case pt_bool: 
    case pt_int:
        char * buff;
        buff = new char(128);
        buff = lltostr(i_val,buff+128);
        s_val = buff;
        delete buff;
        break;
/*    case PTYPE_DATE:
        tm * time = gmtime((long *)(&i_val));
        char tmpbuf[128];
        strftime(tmpbuf, 128,TimeFormat, time);
        s_val = tmpbuf;
        break;*/
    }

    sync = true;
    return s_val;
}
bool Property::convertToBool() 
{
    if (sync) return (bool)i_val;
    bool val;

    switch (type) 
    {
    case pt_str: 
        i_val = atoi(s_val.c_str());
        break;
/*    case PTYPE_DATE:
        throw ConvertException("date","bool");
        break;*/
    }

    val = i_val;
    sync = true;
    return val;
}
time_t Property::convertToDate()
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
/*    case pt_int:
        break;
    case pt_bool:
        throw ConvertException("cannot convert from bool to date","convert exception");
        break;*/
    }


    val = (time_t) i_val;
    sync = true;
    return val;
}

//////////////////////////////////////////////////////GET////////////////////////////////


int64_t Property::getInt() 
{
    if (type == pt_int) return i_val;
    else return convertToInt();
}
const std::string& Property::getStr()
{
    if (type == pt_str) return s_val;
    else return convertToStr();
}
bool Property::getBool()
{
    if (type == pt_bool) return (bool)i_val;
    else return convertToBool();
}
time_t Property::getDate() 
{
    if (type == pt_date) return (time_t)i_val;
    else return convertToDate();
}

//////////////////////////////////////////////////////SET////////////////////////////////

void Property::setInt(int64_t val) 
{
    if (constant) throw ConstantSetException();

    sync = false;
    i_val = val;
}
void Property::setStr(const std::string& val)
{
    if (constant) throw ConstantSetException();

    sync = false;
    s_val = val;
}
void Property::setBool(bool val)
{
    if (constant) throw ConstantSetException();
    sync = false; 
    i_val = val;
}
void Property::setDate(time_t val) 
{
    if (constant) throw ConstantSetException();
    sync = false; 
    i_val = val;
}


/////////////////////////////////////////////////COMPARE///////////////////////////////////////

int Property::Compare(const std::string& val)
{
    return getStr().compare(val);
}

int Property::Compare(bool val)
{
    return (getBool()!=val);
}

int Property::Compare(int val)
{
    int prop = getInt();
    if (prop>val) return 1;
    if (prop<val) return -1;
    return 0;
}

int Property::Compare(Property& val, bool reqcast)
{
    if (!reqcast) 
        if (val.type!=type) throw CompareException("Property: cannot compare incomparable properties");

    return Compare(val, type);
}

int Property::Compare(Property& val,PropertyType pt)
{
    if (pt == pt_int) 
    {
        int pr1 = getInt();
        int pr2 = val.getInt();

        if (pr1>pr2) return 1;
        if (pr1<pr2) return -1;
        return 0;
    }
    else if (pt == pt_bool) 
    {
        return (getBool()!=val.getBool());
    }
    else
        return getStr().compare(val.getStr());
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


void Changeable::changed(const NamedProperty& property)
{
}


}}}
