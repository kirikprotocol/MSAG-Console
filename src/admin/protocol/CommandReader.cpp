#include "CommandReader.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <unistd.h>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/DOMParser.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <xercesc/dom/DOM_Node.hpp>

#include <admin/protocol/CommandGetConfig.h>
#include <admin/protocol/CommandSetConfig.h>
#include <admin/protocol/CommandGetLogs.h>
#include <admin/protocol/CommandGetMonitoringData.h>
#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandShutdown.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandAddService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandListServices.h>
#include <util/Logger.h>
#include <util/xml/DOMErrorLogger.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::util::xml::DOMErrorLogger;

CommandReader::CommandReader(int admSocket)
	: logger(smsc::util::Logger::getCategory("smsc.admin.protocol.CommandReader"))
{
	sock = admSocket;
	parser = createParser();
}

CommandReader::~CommandReader()
{
	delete parser;
	sock = 0;
	parser = 0;
}

Command *CommandReader::read()
	throw (AdminException &)
{
	uint32_t len = readMessageLength();

	XMLByte *buf = new XMLByte [len+1];
	readMessageBody(buf, len);

	// parse message
	MemBufInputSource is(buf, len, "fake_system_id");
	return parseCommand(is);
}

uint32_t CommandReader::readMessageLength()
	throw (AdminException &)
{
	uint32_t len = 0;
	for (int i=0; i<4; i++)
	{
		uint8_t c;
		if (recv(sock, &c, sizeof(c), 0) == 0)
			throw AdminException("Connection broken");
		len = (len << 8) + c;
	}
	return len;
}

void CommandReader::readMessageBody(XMLByte * buf, uint32_t len)
	throw (AdminException &)
{
	buf[len]=0;
	for (int readed=0; readed<len;)
	{
		size_t readed_now = recv(sock, buf+readed, len-readed, 0);
		readed += readed_now;
		if (readed_now == 0)
			throw AdminException("Connection broken");
	}
}

Command* CommandReader::parseCommand(InputSource &source)
	throw (AdminException &)
{

	try
	{
		parser->parse(source);
		if (parser->getErrorCount() > 0)
		{
			logger.warn("%i errors occured in command", parser->getErrorCount());
			throw AdminException("Errors in document");
		}
		DOM_Document data = parser->getDocument();
		
		char * command_name = getCommandName(data);
		Command::Id id = Command::getCommandIdByName(command_name);
		delete[] command_name;

		if (id == Command::undefined)
		{
			logger.warn("Unknown command \"%s\"", command_name);
			throw AdminException("Unknown command");
		}
		return createCommand(id,data);
	}
	catch (const XMLException& e)
	{
		char * message = DOMString(e.getMessage()).transcode();
		XMLExcepts::Codes code = e.getCode();
		unsigned int line = e.getSrcLine();
		logger.warn("An error occured during parsing on line %d. Nested: %d: %s", line, code, message);
		delete[] message;
		throw new AdminException("An errors occured during parsing");
	}
	catch (const DOM_DOMException& e)
	{
		logger.warn("A DOM error occured during parsing command. DOMException code: %i", e.code);
		throw new AdminException("An errors occured during parsing");
	}
	catch (const AdminException & e)
	{
		throw e;
	}
	catch (...)
	{
		logger.warn("An error occured during parsing command");
		throw new AdminException("An errors occured during parsing");
	}
}

char * CommandReader::getCommandName(DOM_Document data)
{
	DOM_NodeList list = data.getElementsByTagName("command");
	if (list.getLength() == 0)
	{
		logger.warn("<command> tag not found in command");
		throw AdminException("<command> tag not found in command");
	}
	DOM_NamedNodeMap attrs = list.item(0).getAttributes();
	DOM_Node name_attr = attrs.getNamedItem("name");
	if (name_attr.isNull())
	{
		logger.warn("<command> tag has not have 'name' attribute");
		throw AdminException("<command> tag has not have 'name' attribute");
	}
	return name_attr.getNodeValue().transcode();
}

Command * CommandReader::createCommand(Command::Id id, DOM_Document data) {
	switch (id)
	{
	case Command::get_config:
		return new CommandGetConfig(data);
	case Command::set_config:
		return new CommandSetConfig(data);
	case Command::get_logs:
		return new CommandGetLogs(data);
	case Command::get_monitoring:
		return new CommandGetMonitoringData(data);
	case Command::start_service:
		return new CommandStartService(data);
	case Command::shutdown_service:
		return new CommandShutdown(data);
	case Command::kill_service:
		return new CommandKillService(data);
	case Command::add_service:
		return new CommandAddService(data);
	case Command::remove_service:
		return new CommandRemoveService(data);
	case Command::list_services:
		return new CommandListServices(data);
	default:
		logger.warn("Unknown command id \"%i\"", id);
		throw AdminException("Unknown command");
	}
}

DOMParser * CommandReader::createParser()
{
	// init parser
	DOMParser *parser = new DOMParser;
	parser->setValidationScheme(DOMParser::Val_Always);
	parser->setDoNamespaces(false);
	parser->setDoSchema(false);
	parser->setValidationSchemaFullChecking(false);
	DOMErrorLogger *errReporter =
		new DOMErrorLogger("smsc.admin.protocol.CommandReader.errReporter");
	parser->setErrorHandler(errReporter);
	parser->setCreateEntityReferenceNodes(false);
	parser->setToCreateXMLDeclTypeNode(false);
	return parser;
}

}
}
}

