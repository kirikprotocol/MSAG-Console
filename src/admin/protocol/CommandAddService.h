#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

#include <admin/protocol/CommandService.h>
#include <util/xml/utilFunctions.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::util::xml::getNodeAttribute;
using smsc::util::xml::getNodeText;

typedef std::pair<int, char *> CmdArgument;

class CommandAddService : public CommandService
{
public:
	CommandAddService(DOM_Document doc) : CommandService(doc)
	{
		setId(add_service);
		
		DOM_Element elem = doc.getDocumentElement();
		DOM_NodeList list = elem.getElementsByTagName("service");
		if (list.getLength() > 0)
		{
			std::vector<CmdArgument> cmd_args;
			DOM_Node node = list.item(0);
			DOM_NodeList childs = node.getChildNodes();
			for (int i=0; i<childs.getLength(); i++)
			{
				DOM_Node param = childs.item(i);
				if (param.getNodeType() == DOM_Node::ELEMENT_NODE)
				{
					std::auto_ptr<char> paramName(param.getNodeName().transcode());
					/*if (strcmp(paramName, "name") == 0)
					{
						serviceName = getNodeText(param);
					}
					else*/ if (strcmp(paramName.get(), "cmd_line") == 0)
					{
						std::auto_ptr<char> text(getNodeText(param));
						cmdLine = text.get();
					}
					else if (strcmp(paramName.get(), "port") == 0)
					{
						std::auto_ptr<char> portStr(getNodeText(param));
						port = atol(portStr.get());
					}
					else if (strcmp(paramName.get(), "arg") == 0)
					{
						char * arg = getNodeText(param);
						std::auto_ptr<char> num(getNodeAttribute(param, "num"));
						cmd_args.push_back(CmdArgument(atoi(num.get()), arg));
					}
				}
			}
			args = new (char*)[cmd_args.size()+1];
			args[cmd_args.size()] = 0;
			for (int i=0; i<cmd_args.size(); i++)
			{
				args[i] = 0;
				for (int j=0; j<cmd_args.size(); j++)
				{
					if (cmd_args[j].first = i)
					{
						args[i] = cmd_args[j].second;
						break;
					}
				}
			}
		}
	}

	const char * const getCmdLine() const
	{
		return cmdLine.c_str();
	}

	const char * const * const getArgs() const
	{
		return args;
	}

	const in_port_t getPort() const
	{
		return port;
	}
protected:
	std::string cmdLine;
	in_port_t port;
	char ** args;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

