#include <memory>
#include "Response.h"
#include <sys/types.h>
#include <string.h>
#include <admin/service/Type.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace protocol {

using namespace smsc::admin::service;
using smsc::util::encode;

static const char * const RESPONSE_HEADER1 =
"<?xml version=\"1.0\" encoding=\"";
static const char* const RESPONSE_HEADER2 = "\"?>\n\
<!DOCTYPE response SYSTEM \"file://response.dtd\">\n\
<response status=\"";
static const size_t RESPONSE_HEADER1_LENGTH = strlen(RESPONSE_HEADER1);
static const size_t RESPONSE_HEADER2_LENGTH = strlen(RESPONSE_HEADER2);
static const char * const RESPONSE_MIDDLE = "\">\n";
static const size_t RESPONSE_MIDDLE_LENGTH = strlen(RESPONSE_MIDDLE);
static const char * const RESPONSE_FOOTER = "</response>";
static const size_t RESPONSE_FOOTER_LENGTH = strlen(RESPONSE_FOOTER);

const Response::_Response
Response::names[Response::response_names_quantity] = {
  {"Ok", Response::Ok},
  {"Error", Response::Error}
};

Response::Response(Status status, const char * const data, const char* encoding)
{
  st = status;
  init(data,encoding);
}

Response::Response(Status status, Variant v, const char* encoding)
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
	const char *type = "unknown";
	std::string str;
	switch (v.getType())
	{
	case service::StringType:
		value = v.getStringValue();
		type = "string";
		break;
	case service::BooleanType:
		value = v.getBooleanValue() ? "true" : "false";
		type = "bool";
		break;
	case service::LongType:
		snprintf(buf, sizeof(buf), "%li", v.getLongValue());
		type = "int";
		break;
	case service::StringListType:
		for (StringList::const_iterator i = v.getStringListValue().begin(); i != v.getStringListValue().end(); i++)
		{
			if (i != v.getStringListValue().begin())
			{
				str += ',';
			}
			str += *i;
		}
		value = str.c_str();
		type = "stringlist";
		break;
	default:
		char buff[1024];
		snprintf(buff, 1000, "Unknown response value type: %u", v.getType());
		throw AdminException(buff);
	}

	std::auto_ptr<char> encodedValue(encode(value));
	std::auto_ptr<char> data(new char[VARIANT_TEMPLET_LENGTH+strlen(type)+strlen(encodedValue.get())+1]);
	snprintf(data.get(), VARIANT_TEMPLET_LENGTH+strlen(type)+strlen(encodedValue.get())+1, "%s%s%s%s%s", VARIANT_HEADER, type, VARIANT_MIDDLE,
					                           encodedValue.get(), VARIANT_FOOTER);
	init(data.get(),encoding);
}


void Response::init(const char * const data, const char* encoding)
{
  const char * const status_name = getStatusName(st);
  if (data != 0)
  {
    text = new char[RESPONSE_HEADER1_LENGTH
                    + strlen(encoding)
                    + RESPONSE_HEADER2_LENGTH
                    + strlen(status_name)
                    + RESPONSE_MIDDLE_LENGTH
                    + strlen(data)
                    + RESPONSE_FOOTER_LENGTH
                    + 1];
  } else {
    text = new char[RESPONSE_HEADER1_LENGTH
                    + strlen(encoding)
                    + RESPONSE_HEADER2_LENGTH
                    + strlen(status_name)
                    + RESPONSE_MIDDLE_LENGTH
                    + RESPONSE_FOOTER_LENGTH
                    + 1];
  }
  strcpy(text, RESPONSE_HEADER1);
  strcat(text, encoding);
  strcat(text, RESPONSE_HEADER2);
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
