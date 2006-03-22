#include <string.h>
#include "Properties.h"
#include <stdlib.h>
#include "scag/exc/SCAGExceptions.h"
#include <memory.h>
#include "util/recoder/recode_dll.h"

#include <iostream>

namespace scag { namespace util { namespace properties 
{

using namespace scag::exceptions;

const char* TimeFormat = "%A, %d - %B - %Y.\n";



//////////////////////////////////////////////////////CONVERT////////////////////////////////

int64_t Property::ConvertUCS2ToInt64(std::string& wstr)
{
    char buff[128];
    int nmax = 128;
    if (wstr.size()/2<128) nmax = wstr.size()/2;

    for (int i = 0; i < nmax; i++) buff[i] = wstr[i*2+1];

    buff[nmax] = 0;

    return atoi(buff);
}

std::string Property::ConvertUCS2ToStr(std::string& wstr)
{
    std::string res;

    char buff[256];
    int nmax = 256;
    if (wstr.size()/2<256) nmax = wstr.size()/2;

    if (nmax < 2) return res;

    for (int i = 0; i < nmax; i++) buff[i] = wstr[i*2+1];

    buff[nmax] = 0;

    res.assign(buff,nmax);
    buff[0] = 0;
    buff[1] = 1;

    res.append(buff,2);

    return res;
}

std::string Property::ConvertInt64ToUCS2(int64_t val)
{
    char * buff = new char(128);
    buff[127] = 0;
    char resbuff[256];

    char * temp = lltostr(i_val,buff+127);
    int size = buff+128 - temp-1;

    for (int i = 0; i < size; i++) 
    {
        resbuff[i*2] = 0;
        resbuff[i*2+1] = temp[i];
    }

    std::string res;
    res.assign(resbuff,size*2);
    resbuff[0] = 0;
    resbuff[1] = 0;
    res.append(resbuff,2);

    //std::cout << "???" << size << std::endl;
    return res;
}


int64_t Property::convertToInt() 
{
    if (sync) return i_val;

    switch (type) 
    {
    case pt_bool: 
        break;
    case pt_str:
        i_val = ConvertUCS2ToInt64(s_val);
        break;
/*    case PTYPE_DATE:
        throw ConvertException("date","int");
        break;*/
    }

    sync = true;
    return i_val;
}
const std::string& Property::convertToStr() 
{
    if (sync) return s_val;

    switch (type) 
    {
    case pt_bool: 
    case pt_int:
        s_val = ConvertInt64ToUCS2(i_val);
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

    char buff[1024];

    switch (type) 
    {
    case pt_str:
        i_val = ConvertUCS2ToInt64(s_val);
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

        strptime(ConvertUCS2ToStr(s_val).c_str(),TimeFormat,&time);
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
    type = pt_int;
}

void Property::setStr(const std::string& val)
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

void AdapterProperty::setStr(const std::string& val)
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

void AdapterProperty::setPureStr(const std::string& val)
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


std::string ConvertStrToWStr(const char * str)
{
    std::string wstr;
    char buff[1024];

    int i;
    for (i=0; ((str[i]!=0)&&(i < 512)); i++) 
    {
        buff[i*2+1] = str[i];
        buff[i*2] = 0;
    }

    wstr.append(buff,i*2);
    buff[0] = 0;
    buff[1] = 0;
    wstr.append(buff,2);

    return wstr;
}

      
std::string ConvertWStrToStr(std::string wstr)
{
    char buff[256];
    int nmax = 256;
    std::string res;

    if (wstr.size()/2<256) nmax = wstr.size()/2;

    if (nmax < 2) return res;
    nmax = nmax - 1;

    for (int i = 0; i < nmax; i++) buff[i] = wstr[i*2+1];

    buff[nmax] = 0;

    res.assign(buff,nmax);

    return res;
}    
           

std::string FormatWStr(std::string& wstr)
{
    std::string str;

    const char * wStrPtr = (char *)wstr.data();
    char buff[2048];
    char temp[32];

    int i;

    if (wstr.size() > 0) str.append("U::");

    for (i=0; i< (wstr.size() / 2) - 1; i++) 
    {
        sprintf(temp,"%d/%d/",wStrPtr[i*2],wStrPtr[i*2+1]);
        str.append(temp);
    }

//    std::cout << "!!!" << str.c_str() << std::endl;

    return str;
}

std::string UnformatWStr(std::string& str)
{
    std::string wstr;
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

            buff[size*2+nByte] = (char)atoi(temp);

            if (nByte == 1) 
            {
                nByte = 0;
                size++;
            } else
                nByte = 1;
        }
    }

    wstr.append((char *)buff,size);
    buff[0] = 0;
    buff[1] = 0;
    wstr.append(buff,2);

    return wstr;
}


}}}
