#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_SERVICE

#include <xercesc/dom/DOM.hpp>
#include "admin/protocol/Command.h"
#include "util/xml/utilFunctions.h"
#include "util/cstrings.h"

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::protocol::Command;
using namespace smsc::util::xml;

class CommandService : public Command
{
public:
	CommandService(Id comId, const xercesc::DOMDocument *doc) throw ()
		: Command(comId)
	{
		setData(doc);
    serviceId.reset(XmlStr(doc->getDocumentElement()->getAttribute(XmlStr("service"))).c_release());
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
