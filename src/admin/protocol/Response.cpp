#include "Response.h"
#include <sys/types.h>
#include <string.h>
#include <admin/service/Type.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::service::StringType;
using smsc::admin::service::BooleanType;
using smsc::admin::service::LongType;

static const char * const RESPONSE_HEADER =
"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n\
<!DOCTYPE response SYSTEM \"file://response.dtd\">\n\
<response status=\"";
static const size_t RESPONSE_HEADER_LENGTH = strlen(RESPONSE_HEADER);
static const char * const RESPONSE_MIDDLE = "\">\n";
static const size_t RESPONSE_MIDDLE_LENGTH = strlen(RESPONSE_MIDDLE);
static const char * const RESPONSE_FOOTER = "</response>";
static const size_t RESPONSE_FOOTER_LENGTH = strlen(RESPONSE_FOOTER);

const Response::_Response
Response::names[Response::response_names_quantity] = {
	{"Ok", Response::Ok},
	{"Error", Response::Error}
};

Response::Response(Status status, const char * const data)
{
	st = status;
	init(data);
}

Response::Response(Status status, Variant v)
	throw (AdminException)
{
	st = status;
	static const char * VARIANT_HEADER = "<variant type=\"";
	static const char * VARIANT_MIDDLE = "\">";
	static const char * VARIANT_FOOTER = "</variant>";
	static const size_t VARIANT_TEMPLET_LENGTH =   strlen(VARIANT_HEADER)
	                                             + strlen(VARIANT_MIDDLE)
	                                             + strlen(VARIANT_FOOTER);
	char buf[sizeof(long)*3+1] = {0};
	const char * value = buf;
	char *type = "unknown";
	switch (v.getType())
	{
	case StringType:
		value = v.getStringValue();
		type = "string";
		break;
	case BooleanType:
		value = v.getBooleanValue() ? "true" : "false";
		type = "bool";
		break;
	case LongType:
		snprintf(buf, sizeof(buf), "%li", v.getLongValue());
		type = "int";
		break;
	default:
		throw AdminException("Unknown response value type");
	}

	std::auto_ptr<char> data(new char[VARIANT_TEMPLET_LENGTH+strlen(type)+strlen(value)+1]);
	snprintf(data.get(), VARIANT_TEMPLET_LENGTH+strlen(type)+strlen(value)+1, "%s%s%s%s%s", VARIANT_HEADER, type, VARIANT_MIDDLE,
					                           value, VARIANT_FOOTER);
	init(data.get());
}


void Response::init(const char * const data)
{
	const char * const status_name = getStatusName(st);
	if (data != 0)
	{
		text = new char[RESPONSE_HEADER_LENGTH
										+ strlen(status_name)
										+ RESPONSE_MIDDLE_LENGTH
										+ strlen(data)
										+ RESPONSE_FOOTER_LENGTH
										+ 1];
	} else {
		text = new char[RESPONSE_HEADER_LENGTH
										+ strlen(status_name)
										+ RESPONSE_MIDDLE_LENGTH
										+ RESPONSE_FOOTER_LENGTH
										+ 1];
	}
	strcpy(text, RESPONSE_HEADER);
	strcat(text, status_name);
	strcat(text, RESPONSE_MIDDLE);
	if (data != 0)
	{
		strcat(text, data);
	}
	strcat(text, RESPONSE_FOOTER);
}

Response::~Response() {
	delete[] text;
}

const char * const Response::getStatusName(Status status)
{
	for (int i=0; i<response_names_quantity; i++)
	{
		if (names[i].id == status)
		{
			return names[i].name;
		}
	}
	return "undefined";
}

}
}
}

