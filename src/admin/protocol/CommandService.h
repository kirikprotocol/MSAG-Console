#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_SERVICE

#include <admin/protocol/Command.h>
#include <util/xml/utilFunctions.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::protocol::Command;
using smsc::util::xml::getNodeText;

class CommandService : public Command
{
public:
	CommandService(Id comId, DOM_Document doc) throw ()
		: Command(comId)
	{
		serviceName = 0;
		setData(doc);
		serviceName = doc.getDocumentElement().getAttribute("service").transcode();
	}

	virtual ~CommandService()
		throw()
	{
		if (serviceName != 0)
		{
			delete serviceName;
		}
	}

	const char * const getServiceName() const throw ()
	{
		return serviceName;
	}

protected:
	char * serviceName;
};

}
}
}

#endif
