#include <admin/service/Variant.h>

#include <admin/AdminException.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::util::cStringCopy;
using smsc::admin::AdminException;

Variant::Variant(const Type variant_type)
	throw (AdminException)
{
  type = variant_type;
  switch (type)
  {
  case service::StringType:
    strValue = "";
    break;
  case service::LongType:
    longValue = 0;
    break;
  case service::BooleanType:
    boolValue = false;
    break;
  case service::StringListType:
    stringListValue = new StringList;
		break;
  default:
    type = undefined;
		longValue = 0;
    throw AdminException("Unknow type of variant to create");
  }
}

Variant::Variant(const char * const value) 
  : type(StringType), strValue(cStringCopy(value))
{}

Variant::Variant(const long value) 
  : type(LongType), longValue(value)
{}

Variant::Variant(const bool value)
  : type(BooleanType), boolValue(value)
{}

Variant::Variant(const StringList & value)
  : type(StringListType)
{
  stringListValue = new StringList;
  for (StringList::const_iterator i = value.begin(); i != value.end(); i++)
  {
    stringListValue->push_back(cStringCopy(*i));
  }
}

Variant::Variant()
{
  type = LongType;
  longValue = 0;
}

void appendStringList(StringList * stringList, const char * const value)
{
  std::auto_ptr<char> buffer(new char[strlen(value)+1]);
  char * bptr = buffer.get();
  for (const char * ptr = value; *ptr != 0; ptr++)
  {
    if (*(ptr) == ',')
    {
      *(bptr) = 0;
      stringList->push_back(cStringCopy(buffer.get()));
      bptr = buffer.get();
      continue;
    }
    if ((*(ptr) == '\\') && (*(ptr+1) != 0))
    {
      ptr++;
    }
    *(bptr++) = *(ptr);
  }
  *(bptr) = 0;
  stringList->push_back(cStringCopy(buffer.get()));
}

Variant::Variant(const char * const value, Type variant_type)
	throw (AdminException)
{
  type = variant_type;
  switch (type)
  {
  case StringType:
    strValue = cStringCopy(value);
    break;
  case LongType:
    longValue = atol(value);
		break;
  case BooleanType:
    boolValue =    (strcmp(value,  "true") == 0)
                || (strcmp(value,  "on") == 0)
                || (strcmp(value,  "1") == 0)
                || (strcmp(value,  "-1") == 0)
                || (strcmp(value,  "yes") == 0);
		break;
  case service::StringListType:
    appendStringList(stringListValue = new StringList, value);
	  break;
  default:
    type = undefined;
    throw AdminException("Unknow type of variant to create");
  }
}


Variant::Variant(const Variant &copy)
  throw (AdminException)
{
  initByCopy(copy);
}

Variant::Variant & Variant::operator = (const Variant & copy)
  throw (AdminException)
{
  initByCopy(copy);
  return *this;
}

Variant::~Variant()
{
  if (type == StringType && strValue != 0)
  {
    delete strValue;
    strValue = 0;
  }
  if (type == StringListType)
  {
    for (StringList::const_iterator i = stringListValue->begin(); i != stringListValue->end(); i++)
    {
      if (*i != 0)
        delete *i;
    }
    delete stringListValue;
  }
}

const Type Variant::getType() const
{
  return type;
}

const char * const Variant::getStringValue() const
  throw (AdminException)
{
  if (type != StringType)
    throw AdminException("Illegal parameter type");
  return strValue;
}

const long Variant::getLongValue() const
  throw (AdminException)
{
  if (type != LongType)
    throw AdminException("Illegal parameter type");
  return longValue;
}

const bool Variant::getBooleanValue() const
  throw (AdminException)
{
  if (type != BooleanType)
    throw AdminException("Illegal parameter type");
  return boolValue;
}

const StringList& Variant::getStringListValue() const
  throw (AdminException)
{
  if (type != StringListType)
    throw AdminException("Illegal parameter type");
  return *stringListValue;
}

char * quoteCommas(const char * const value)
{
	std::string result;
	for (const char * i = value; *i != 0; i++)
	{
		if ((*i == ',') || (*i == '\\'))
			result +=	'\\';
		result += *i;
	}
	return cStringCopy(result.c_str());
}

void Variant::appendValueToStringList(const char * const value) 
	throw (AdminException)
{
  if (type != StringListType)
    throw AdminException("Illegal parameter type");
	stringListValue->push_back(quoteCommas(value));
}


void Variant::initByCopy(const Variant& copy)
  throw (AdminException)
{
  type = copy.type;
  switch (type)
  {
  case StringType:
    strValue = cStringCopy(copy.strValue);
    break;
  case LongType:
    longValue = copy.longValue;
    break;
  case BooleanType:
    boolValue = copy.boolValue;
    break;
  case StringListType:
    stringListValue = new StringList;
    for (StringList::const_iterator i = copy.stringListValue->begin(); i != copy.stringListValue->end(); i++)
    {
      stringListValue->push_back(cStringCopy(*i));
    }
    break;
  default:
    throw AdminException("Unknow type of variant to copy");
  }
}


}
}
}
