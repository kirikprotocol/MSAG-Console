#ifndef SMSC_ADMIN_PROTOCOL_RESPONSE
#define SMSC_ADMIN_PROTOCOL_RESPONSE

namespace smsc {
namespace admin {
namespace protocol {

class Response
{
public:
	enum Status {
		Ok, Error
	};

	Response(Status status, const char * const data);
	~Response();
	const char * const getText() {return text;};
	const char * const getStatusName(Status status);

protected:
	struct _Response
	{
		char * name;
		Status id;
	};

	static const int response_names_quantity = 2;
	static const _Response names[response_names_quantity];
	Status st;
	char * text;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_RESPONSE
