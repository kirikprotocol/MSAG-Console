#include "CommandReader.h"

#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/DOMParser.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>
#include <xercesc/dom/DOM_NamedNodeMap.hpp>
#include <xercesc/dom/DOM_Node.hpp>

#include <admin/protocol/CommandStartService.h>
#include <admin/protocol/CommandShutdown.h>
#include <admin/protocol/CommandKillService.h>
#include <admin/protocol/CommandAddService.h>
#include <admin/protocol/CommandRemoveService.h>
#include <admin/protocol/CommandCall.h>
#include <admin/protocol/CommandListServices.h>
#include <admin/protocol/CommandListComponents.h>
#include <core/network/Socket.hpp>
#include <util/Logger.h>
#include <util/xml/DOMErrorLogger.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::util::xml::DOMErrorLogger;
using smsc::core::network::Socket;
using smsc::util::Logger;

CommandReader::CommandReader(Socket * admSocket)
	: logger(Logger::getCategory("smsc.admin.protocol.CommandReader"))
{
	sock = admSocket;
	parser = createParser();
}

CommandReader::~CommandReader()
{
	delete parser->getErrorHandler();
	delete parser;
	sock = 0;
	parser = 0;
}

Command *CommandReader::read()
	throw (AdminException)
{
	uint32_t len = readMessageLength();

	std::auto_ptr<XMLByte> buf(new XMLByte [len+1]);
	readMessageBody(buf.get(), len);

	// parse message
	MemBufInputSource is(buf.get(), len, "received_command.xml");
	return parseCommand(is);
}

uint32_t CommandReader::readMessageLength()
	throw (AdminException)
{
	uint32_t len = 0;
	for (int i=0; i<4; i++)
	{
		int c = sock->readChar();
		if (c == -1)
			throw AdminException("Connection broken on reading message length");
		else
			len = (len << 8) + (uint8_t)c;
	}
	return len;
}

void CommandReader::readMessageBody(XMLByte * buf, uint32_t len)
	throw (AdminException)
{
	buf[len]=0;
	for (long readed=0; readed<(long)len;)
	{
		int readed_now = sock->Read((char *)(buf+readed), len-readed);
		if (readed_now == 0 || readed_now == -1)
			throw AdminException("Connection broken on reading message body");
		readed += readed_now;
	}
}

Command* CommandReader::parseCommand(InputSource &source)
	throw (AdminException)
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
		throw AdminException("An errors occured during parsing");
	}
	catch (const DOM_DOMException& e)
	{
		logger.warn("A DOM error occured during parsing command. DOMException code: %i", e.code);
		throw AdminException("An errors occured during parsing");
	}
	catch (const AdminException & e)
	{
		throw;
	}
	catch (...)
	{
		logger.warn("An error occured during parsing command");
		throw AdminException("An errors occured during parsing");
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
/*	case Command::get_config:
		return new CommandGetConfig(data);
	case Command::set_config:
		return new CommandSetConfig(data);
	case Command::get_logs:
		return new CommandGetLogs(data);
	case Command::get_monitoring:
		return new CommandGetMonitoringData(data);*/
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
	case Command::call:
		return new CommandCall(data);
	case Command::list_components:
		return new CommandListComponents(data);
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

