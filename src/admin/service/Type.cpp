#include "Type.h"

#include <string>
#include <admin/AdminException.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;

Type cstr2Type(const char * const typestr)
{
	if (strcmp(typestr, "string") == 0)
	{
		return StringType;
	}
	else if (strcmp(typestr, "int") == 0)
	{
		return LongType;
	}
	else if (strcmp(typestr, "bool") == 0)
	{
		return BooleanType;
	}
	else
	{
		return undefined;
	}
}

const char * const type2cstr(Type type)
{
	switch (type)
	{
	case StringType:
		return "string";
		break;
	case LongType:
		return "int";
		break;
	case BooleanType:
		return "bool";
		break;
	default:
		return 0;
	}
}

}
}
}
