#include "Response.h"
#include <sys/types.h>
#include <string.h>

namespace smsc {
namespace admin {
namespace protocol {

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

