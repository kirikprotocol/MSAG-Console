#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_SERVICE

#include <admin/protocol/Command.h>
#include <util/xml/utilFunctions.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::protocol::Command;
using smsc::util::xml::getNodeText;

class CommandService : public Command
{
public:
	CommandService(DOM_Document doc) : Command()
	{
		setId(undefined);
		setData(doc);

		DOM_Element elem = doc.getDocumentElement();
		DOM_NodeList list = elem.getElementsByTagName("service");
		if (list.getLength() > 0)
		{
			DOM_NodeList childs = list.item(0).getChildNodes();
			for (unsigned i=0; i<childs.getLength(); i++)
			{
				DOM_Node child = childs.item(i);
				if (child.getNodeType() == DOM_Node::ELEMENT_NODE)
				{
					std::auto_ptr<char> nodeName(child.getNodeName().transcode());
					if (std::strcmp(nodeName.get(), "name") == 0)
					{
						std::auto_ptr<char> name(getNodeText(child));
						serviceName = name.get();
					}
				}
			}
		}
	}

	const char * const getServiceName() const
	{
		return serviceName.c_str();
	}

protected:
	std::string serviceName;
};

}
}
}

#endif
