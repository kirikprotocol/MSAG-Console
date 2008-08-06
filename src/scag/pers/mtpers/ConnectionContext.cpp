#include "ConnectionContext.h"
#include "IOTask.h"

namespace scag { namespace mtpers { 


ConnectionContext::ConnectionContext(Socket* sock):socket(sock), action(READ_REQUEST), packetLen(0),
                                                   iotask(NULL), packet(NULL)
{
  if (socket) {
    SocketData::setContext(socket, this);
    SocketData::setSocketState(socket, READ_SOCKET);
  }
  logger = Logger::getInstance("context");
}

ConnectionContext::~ConnectionContext() {
  if (socket) {
    delete socket;
  }
  if (packet) {
    delete packet;
  }
}

void ConnectionContext::clearBuffers() {
  inbuf.Empty();
  outbuf.Empty();
  outbuf.SetPos(PACKET_LENGTH_SIZE);
}

void ConnectionContext::createFakeResponse(PersServerResponseType response) {
  smsc_log_debug(logger, "%p: Create Fake response %d", this, response);
  outbuf.Empty();
  outbuf.SetPos(0);
  outbuf.WriteInt32(sizeof(uint8_t) + PACKET_LENGTH_SIZE);
  outbuf.WriteInt8(response);
  outbuf.setPos(0);
  action = SEND_RESPONSE;
  SocketData::setSocketState(socket, WRITE_SOCKET);
}

bool ConnectionContext::notSupport(PersCmd cmd) {
  return (cmd > scag::pers::PC_MTBATCH || cmd == scag::pers::PC_BATCH
          || cmd == scag::pers::PC_TRANSACT_BATCH) ? true : false;
}

bool ConnectionContext::parsePacket() {
  PersServerResponseType response = scag::pers::RESPONSE_BAD_REQUEST;
  try {
    PersCmd cmd = (PersCmd)inbuf.ReadInt8();
    if (cmd == scag::pers::PC_PING) {
      smsc_log_debug(logger, "Ping received");
      createFakeResponse(scag::pers::RESPONSE_OK);
      return false;
    }
    if (notSupport(cmd)) {
      createFakeResponse(scag::pers::RESPONSE_NOTSUPPORT);
      return false;
    }
    if (packet) {
      delete packet;
    }
    if (cmd == scag::pers::PC_MTBATCH) {
      smsc_log_debug(logger, "Batch received");
      packet = new BatchPacket();
    } else {
      smsc_log_debug(logger, "Command %d received", cmd);
      packet = new CommandPacket(cmd);
    }
    packet->deserialize(inbuf);
    action = PROCESS_REQUEST;
    return true;
  } catch(const SerialBufferOutOfBounds& e) {
    smsc_log_warn(logger, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s",
                   inbuf.length(), inbuf.toString().c_str());
  } catch(const std::runtime_error& e) {
    smsc_log_warn(logger, "std::runtime_error: Error profile key: %s. received buffer len=%d, data=%s",
                   e.what(), inbuf.length(), inbuf.toString().c_str());
  } catch(const PersCommandNotSupport& e) {
    smsc_log_warn(logger, "std::runtime_error: Not support command. received buffer len=%d, data=%s",
                   inbuf.length(), inbuf.toString().c_str());
    response = scag::pers::RESPONSE_NOTSUPPORT;
  }
  createFakeResponse(response);
  return false;
}

void ConnectionContext::sendResponse() {
  action = SEND_RESPONSE;
  SocketData::setSocketState(socket, WRITE_SOCKET);
  iotask->addSocket(socket);
}

void ConnectionContext::changeSocketState() {
  iotask->changeSocketState(socket);
}


}
}

