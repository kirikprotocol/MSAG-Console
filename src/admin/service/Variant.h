#ifndef SMSC_ADMIN_SERVICE_VARIANT
#define SMSC_ADMIN_SERVICE_VARIANT

#include <admin/AdminException.h>
#include <admin/service/Type.h>
#include <core/buffers/Hash.hpp>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::util::cStringCopy;
using smsc::core::buffers::Hash;
using smsc::admin::AdminException;

class Variant
{
private:
	Type type;
	union {
		char * strValue;
		long longValue;
		bool boolValue;
	};
public:
	Variant(const char * const value)
	{
		type = StringType;
		strValue = cStringCopy(value);
	}

	Variant(const long value)
	{
		type = LongType;
		longValue = value;
	}

	Variant(const bool value)
	{
		type = BooleanType;
		boolValue = value;
	}

	Variant()
	{
		type = LongType;
		longValue = 0;
	}

	Variant(const Variant &copy)
		throw (AdminException)
	{
		initByCopy(copy);
	}

	Variant & operator = (const Variant & copy)
	{
		initByCopy(copy);
		return *this;
	}

	virtual ~Variant()
	{
		if (type == StringType && strValue != 0)
		{
			delete strValue;
			strValue = 0;
		}
	}

	const Type getType() const
	{
		return type;
	}

	const char * const getStringValue() const
	{
		if (type != StringType)
			throw AdminException("Illegal parameter type");
		return strValue;
	}

	long getLongValue() const
	{
		if (type != LongType)
			throw AdminException("Illegal parameter type");
		return longValue;
	}

	bool getBooleanValue() const
	{
		if (type != BooleanType)
			throw AdminException("Illegal parameter type");
		return boolValue;
	}

protected:
	void initByCopy(const Variant& copy)
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
		default:
			throw AdminException("Unknow type of variant to copy");
		}
	}
};

typedef Hash<Variant> Arguments;

}
}
}

#endif //ifndef SMSC_ADMIN_SERVICE_VARIANT
