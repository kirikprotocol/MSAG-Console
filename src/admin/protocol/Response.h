#ifndef SMSC_ADMIN_PROTOCOL_RESPONSE
#define SMSC_ADMIN_PROTOCOL_RESPONSE

#include <admin/service/Variant.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::service::Variant;

class Response
{
public:
	enum Status {
		Ok, Error
	};

	Response(Status status, const char * const data);
	Response(Status status, Variant v) throw (AdminException &);
	~Response();
	const char * const getText() {return text;};
	const char * const getStatusName(Status status);

protected:
	void init(const char * const data);

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
