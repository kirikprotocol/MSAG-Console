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
#include <admin/protocol/CommandSetServiceStartupParameters.h>
#include <core/network/Socket.hpp>
#include <logger/Logger.h>
#include <util/xml/DOMErrorLogger.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::util::xml::DOMErrorLogger;
using smsc::core::network::Socket;
using smsc::logger::Logger;

CommandReader::CommandReader(Socket * admSocket)
	: logger(Logger::getInstance("smsc.admin.protocol.CommandReader"))
{
	sock = admSocket;
}

CommandReader::~CommandReader()
{
	sock = 0;
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
		DOM_Document data = reader.read(source);
		
		std::auto_ptr<char> command_name(getCommandName(data));
		Command::Id id = Command::getCommandIdByName(command_name.get());

		if (id == Command::undefined)
		{
			logger.warn("Unknown command \"%s\"", command_name.get());
			throw AdminException("Unknown command");
		}
		return createCommand(id,data);
	}
	catch (const DOMTreeReader::ParseException &e)
	{
		logger.warn("A parse error occured during parsing command: %s", e.what());
		throw AdminException("An errors occured during parsing: %s", e.what());
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
	throw AdminException("Fatal error: unreachible code reached in smsc::admin::protocol::CommandReader.parseCommand(InputSource &)");
}

char * CommandReader::getCommandName(DOM_Document data)
{
	DOM_Element commandElem = data.getDocumentElement();
	if (!commandElem.getNodeName().equals("command"))
	{
		logger.warn("<command> tag not found in command");
		throw AdminException("<command> tag not found in command");
	}
	return commandElem.getAttribute("name").transcode();
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
	case Command::set_service_startup_parameters:
		return new CommandSetServiceStartupParameters(data);
	default:
		logger.warn("Unknown command id \"%i\"", id);
		throw AdminException("Unknown command");
	}
}

}
}
}

