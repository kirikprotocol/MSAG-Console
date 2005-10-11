#include "CommandReader.h"

#include <xercesc/dom/DOM.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>

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
#include <util/xml/DOMTreeReader.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::util::xml::DOMErrorLogger;
using smsc::core::network::Socket;
using smsc::logger::Logger;
using namespace xercesc;

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

  if(len > 10 * 1024 * 1024)
      throw AdminException("Too big message length( more then > 10 Mb)");

  std::auto_ptr<XMLByte> buf(new XMLByte [len+1]);
  readMessageBody(buf.get(), len);

  smsc_log_debug(logger, "Received command:\n%s", buf.get());
  // parse message
  Wrapper4InputSource is(new MemBufInputSource (buf.get(), len, "received_command.xml"));
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

int CommandReader::getCommandIdByName(const char * const command_name)
{
  return Command::getCommandIdByName(command_name);
}

Command* CommandReader::parseCommand(DOMInputSource &source)
  throw (AdminException)
{
  try
  {
    smsc::util::xml::DOMTreeReader reader;
    DOMDocument *data = reader.read(source);

    std::auto_ptr<char> command_name(getCommandName(data));
    int id = getCommandIdByName(command_name.get());

    if (id == Command::undefined)
    {
      smsc_log_warn(logger, "Unknown command \"%s\"", command_name.get());
      throw AdminException("Unknown command");
    }

    return createCommand(id,data);
  }
  catch (const ParseException &e)
  {
    smsc_log_warn(logger, "A parse error occured during parsing command: %s", e.what());
    throw AdminException("An errors occured during parsing: %s", e.what());
  }
  catch (const XMLException& e)
  {
    XMLExcepts::Codes code = e.getCode();
    unsigned int line = e.getSrcLine();
    smsc_log_warn(logger, "An error occured during parsing on line %d. Nested: %d: %s", line, code, XmlStr(e.getMessage()).c_str());
    throw AdminException("An errors occured during parsing");
  }
  catch (const DOMException& e)
  {
    smsc_log_warn(logger, "A DOM error occured during parsing command. DOMException code: %i", e.code);
    throw AdminException("An errors occured during parsing");
  }
  catch (const AdminException & e)
  {
    throw;
  }
  catch (...)
  {
    smsc_log_warn(logger, "An error occured during parsing command");
    throw AdminException("An errors occured during parsing");
  }
  throw AdminException("Fatal error: unreachible code reached in smsc::admin::protocol::CommandReader.parseCommand(InputSource &)");
}

char * CommandReader::getCommandName(const DOMDocument *data)
{
  DOMElement *commandElem = data->getDocumentElement();
  if (!commandElem)
  {
    smsc_log_error(logger, "DTD file for command not found");
    throw AdminException("DTD file for command not found");
  }
  if (XMLString::compareString(commandElem->getNodeName(), XmlStr("command")) != 0)
  {
    smsc_log_warn(logger, "<command> tag not found in command");
    throw AdminException("<command> tag not found in command");
  }
  return XmlStr(commandElem->getAttribute(XmlStr("name"))).c_release();
}

Command * CommandReader::createCommand(int id, const DOMDocument *data) {
  switch (id)
  {
/*  case Command::get_config:
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
    smsc_log_warn(logger, "Unknown command id \"%i\"", id);
    throw AdminException("Unknown command");
  }
}

}
}
}
