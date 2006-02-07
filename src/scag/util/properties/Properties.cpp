#include <string.h>
#include "Properties.h"
#include <stdlib.h>
#include "scag/exc/SCAGExceptions.h"
#include <memory.h>

namespace scag { namespace util { namespace properties 
{

using namespace scag::exceptions;

const char* TimeFormat = "%A, %d - %B - %Y.\n";



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
        i_val = atoi(ConvertWStrToStr(s_val).c_str());
        break;
/*    case PTYPE_DATE:
        throw ConvertException("date","int");
        break;*/
    }

    val = i_val;
    sync = true;
    return val;
}
const std::wstring& Property::convertToStr() 
{
    if (sync) return s_val;

    switch (type) 
    {
    case pt_bool: 
    case pt_int:
        char * buff;
        buff = new char(128);
        buff[127] = 0;

        char * temp = lltostr(i_val,buff+127);

        s_val = ConvertStrToWStr(temp);

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
        i_val = atoi(ConvertWStrToStr(s_val).c_str());
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
        strptime(ConvertWStrToStr(s_val).c_str(),TimeFormat,&time);
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
const std::wstring& Property::getStr()
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
    type = pt_int;
}

void Property::setStr(const std::wstring& val)
{
    if (constant) throw ConstantSetException();
    sync = false;
    s_val = val;
    type = pt_str;
}

void Property::setBool(bool val)
{
    if (constant) throw ConstantSetException();
    sync = false; 
    i_val = val;
    type = pt_bool;
}
void Property::setDate(time_t val) 
{
    if (constant) throw ConstantSetException();
    sync = false; 
    i_val = val;
    type = pt_date;
}


/////////////////////////////////////////////////COMPARE///////////////////////////////////////

int Property::Compare(const std::wstring& val)
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

void AdapterProperty::setStr(const std::wstring& val)
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

void AdapterProperty::setPureStr(const std::wstring& val)
{
    Property::setStr(val);
}
void AdapterProperty::setPureInt(int64_t val)
{
    Property::setInt(val);
}
void AdapterProperty::setPureBool(bool val)
{
    Property::setBool(val);
}
void AdapterProperty::setPureDate(time_t val)
{
    Property::setDate(val);
}


////////////////////////////////////PROPERTY MANAGER/////////////////////////
/*NamedProperty* PropertyManager::getProperty(const std::string& name)
{
    return 0;
} */


std::wstring ConvertStrToWStr(const char * str)
{
    std::wstring wstr;
    wchar_t buff[1024];

    int i;
    for (i=0; str[i]!=0; i++) 
    {
        wchar_t wchar;

        int res = mbtowc(&wchar,&str[i],MB_CUR_MAX);
        if (res <= 0) return wstr;

        buff[i] = wchar;
    }
    wstr.append(buff,i);
    return wstr;
}


std::string ConvertWStrToStr(std::wstring wstr)
{
    std::string str;
    char buff[1024];

    int i;

    for (i=0; i < wstr.size();i++) 
    {
        char chr;

        int res = wctomb(&chr,wstr[i]);

        if (res <= 0) 
        {
            return str;
        }

        buff[i] = chr;
    }
    str.append(buff,i);

    return str;
}


std::string FormatWStr(std::wstring& wstr)
{
    std::string str;

    const char * wStrPtr = (char *)wstr.c_str();
    char buff[1024];
    int i;

    if (wstr.size() > 0) str.append("U::");

    for (i=0; i<wstr.size(); i++) 
    {
        sprintf(buff,"%d/%d/",wStrPtr[i*4+2],wStrPtr[i*4+3]);
        str.append(buff);
    }

    return str;
}

std::wstring UnformatWStr(std::string& str)
{
    std::wstring wstr;
    if (str.size()<3) return wstr;
    if (str.substr(0,3)!="U::") return wstr;

    char buff[1024];
    memset(buff,0,1024);

    char temp[32];

    int offset = 0;
    int size = 0;
    int nByte = 0;

    for (int i=3;i<str.size();i++) 
    {
        if (str[i]!='/') 
        {
            temp[offset] = str[i];
            offset++;
        }
        else 
        {
            temp[offset] = 0;
            offset = 0;

            buff[size*4+2+nByte] = (char)atoi(temp);

            if (nByte == 1) 
            {
                nByte = 0;
                size++;
            } else
                nByte = 1;
        }
    }

    wstr.append((wchar_t *)buff,size);

    return wstr;
}


}}}
