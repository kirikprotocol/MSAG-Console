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
		setData(doc);
		serviceId.reset(doc.getDocumentElement().getAttribute("service").transcode());
	}

	virtual ~CommandService()
	{
	}

	const char * const getServiceId() const throw ()
	{
		return serviceId.get();
	}
	
protected:
	std::auto_ptr<char> serviceId;
};

}
}
}

#endif
