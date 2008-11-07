#include "ConnectionContext.h"
#include "WriterTaskManager.h"
#include "ReaderTaskManager.h"

namespace scag { namespace mtpers { 

using smsc::core::synchronization::MutexGuard;
const size_t MAX_PACKET_SIZE = 100000;

ConnectionContext::ConnectionContext(Socket* sock, WriterTaskManager& writerManager, ReaderTaskManager& readerManager)
                   :socket_(sock), action_(READ_REQUEST), packetLen_(0), packet_(NULL), writerManager_(writerManager), 
                    readerManager_(readerManager), tasksCount(1)
{
  if (socket_) {
    SocketData::setContext(socket_, this);
  }
  logger_ = Logger::getInstance("context");
}

ConnectionContext::~ConnectionContext() {
  if (socket_) {
    delete socket_;
  }
  if (packet_) {
    delete packet_;
  }
}

void ConnectionContext::createFakeResponse(PersServerResponseType response) {
  smsc_log_debug(logger_, "%p: Create Fake response %d", this, response);
  action_ = SEND_RESPONSE;
  writeData(reinterpret_cast<const char*>(&response), static_cast<uint32_t>(sizeof(uint8_t)));
}

bool ConnectionContext::notSupport(PersCmd cmd) {
  return (cmd > scag::pers::util::PC_MTBATCH || cmd == scag::pers::util::PC_BATCH
          || cmd == scag::pers::util::PC_TRANSACT_BATCH) ? true : false;
}

bool ConnectionContext::parsePacket() {
  PersServerResponseType response = scag::pers::util::RESPONSE_BAD_REQUEST;
  try {
    PersCmd cmd = (PersCmd)inbuf_.ReadInt8();
    if (cmd == scag::pers::util::PC_PING) {
      smsc_log_debug(logger_, "Ping received");
      createFakeResponse(scag::pers::util::RESPONSE_OK);
      return false;
    }
    if (notSupport(cmd)) {
      createFakeResponse(scag::pers::util::RESPONSE_NOTSUPPORT);
      return false;
    }
    if (packet_) {
      delete packet_;
    }
    if (cmd == scag::pers::util::PC_MTBATCH) {
      smsc_log_debug(logger_, "Batch received");
      packet_ = new BatchPacket(this);
    } else {
      smsc_log_debug(logger_, "Command %d received", cmd);
      packet_ = new CommandPacket(this, cmd);
    }
    packet_->deserialize(inbuf_);
    action_ = PROCESS_REQUEST;
    return true;
  } catch(const SerialBufferOutOfBounds& e) {
    smsc_log_warn(logger_, "SerialBufferOutOfBounds Bad data in buffer received len=%d, data=%s",
                   inbuf_.length(), inbuf_.toString().c_str());
  } catch(const std::runtime_error& e) {
    smsc_log_warn(logger_, "std::runtime_error: Error profile key: %s. received buffer len=%d, data=%s",
                   e.what(), inbuf_.length(), inbuf_.toString().c_str());
  } catch(const PersCommandNotSupport& e) {
    smsc_log_warn(logger_, "std::runtime_error: Not support command. received buffer len=%d, data=%s",
                   inbuf_.length(), inbuf_.toString().c_str());
    response = scag::pers::util::RESPONSE_NOTSUPPORT;
  }
  createFakeResponse(response);
  return false;
}

void ConnectionContext::sendFakeResponse() {
  if (writerManager_.process(this)) {
    ++tasksCount;
  }
}

void ConnectionContext::sendResponse() {
  {
    MutexGuard mg(mutex_);
    action_ = SEND_RESPONSE;
    writeData(packet_->getResponseData(), packet_->getResponseSize());
  }
  bool canProcess = writerManager_.process(this);
  if (!canProcess) {
    //TODO: error, response must be processed
    MutexGuard mg(mutex_);
    --tasksCount;
  }
}

void ConnectionContext::writeData(const char* data, uint32_t size) {
  uint32_t pos = outbuf_.getPos();
  uint32_t bufSize = outbuf_.GetSize();
  outbuf_.SetPos(bufSize);
  outbuf_.WriteInt32(size + PACKET_LENGTH_SIZE);
  outbuf_.Append(data, size);
  outbuf_.SetPos(pos);
}


bool ConnectionContext::processReadSocket() {
  bool parseResult = false;
  {
    MutexGuard mg(mutex_);
    if (action_ != READ_REQUEST) {
      smsc_log_warn(logger_, "cx:%p socket %p error action=%d, must be READ_REQUEST", this, socket_, action_);
      return false;
    }
    SerialBuffer& sb = inbuf_;
  
    if(sb.GetSize() < PACKET_LENGTH_SIZE) {
      int n = socket_->Read(readBuf_, PACKET_LENGTH_SIZE - sb.GetSize());
      smsc_log_debug(logger_, "cx:%p read(len) %u bytes from %p", this, n, socket_);
      if (n <= 0) {
        if (n) smsc_log_warn(logger_, "%p: read error: %s(%d)", this, strerror(errno), errno);
        return false;
      }
      sb.Append(readBuf_, n);
      if (sb.GetSize() < PACKET_LENGTH_SIZE) {
        SocketData::updateTimestamp(socket_, time(NULL));
        return true;
      }
      n = sb.getPos();
      sb.SetPos(0);
      packetLen_ = sb.ReadInt32();
      smsc_log_debug(logger_, "%d bytes will be read from %p", packetLen_, socket_);
      if(packetLen_ > MAX_PACKET_SIZE) {
        smsc_log_warn(logger_, "Too big packet from client");
        return false;
      }
      sb.SetPos(n);
    } 
    int n = packetLen_ - sb.GetSize();
    n = socket_->Read(readBuf_, n > READ_BUF_SIZE ? READ_BUF_SIZE : n);
  
    smsc_log_debug(logger_, "read %u bytes from %p", n, socket_);
  
    if (n > 0) {
      SocketData::updateTimestamp(socket_, time(NULL));
      sb.Append(readBuf_, n);
    } else if(errno != EWOULDBLOCK) {
      if (n) smsc_log_warn(logger_, "read error: %s(%d)", strerror(errno), errno);
      return false;
    }
    if (sb.GetSize() < packetLen_) {
      return true;
    }
    smsc_log_debug(logger_, "read from socket:%p len=%d, data=%s", socket_, sb.length(), sb.toString().c_str());
    inbuf_.SetPos(PACKET_LENGTH_SIZE);
    parseResult = parsePacket();
    ++tasksCount;
    inbuf_.Empty();
  }
  //
  if (!parseResult) {
    if (!writerManager_.process(this)) {
      MutexGuard mg(mutex_);
      --tasksCount;
      //ERROR!!!
    }
    return true;
  }
  if (!readerManager_.processPacket(packet_)) {
    {
      MutexGuard mg(mutex_);
      createFakeResponse(scag::pers::util::RESPONSE_ERROR);
    }
    if (!writerManager_.process(this)) {
      MutexGuard mg(mutex_);
      --tasksCount;
      //ERROR!!!
    }
  }
  return true;
}

bool ConnectionContext::processWriteSocket() {
  MutexGuard mg(mutex_);
  if (action_ != SEND_RESPONSE) {
    return true;
  }
  SerialBuffer& sb = outbuf_;
  uint32_t len = sb.GetSize();
  if (len == 0) {
    return true;
  }
  smsc_log_debug(logger_, "write %u bytes to %p, GetCurPtr: %x, GetPos: %d data=%s",
                  len, socket_, sb.GetCurPtr(), sb.GetPos(), sb.toString().c_str());

  int n = socket_->Write(sb.GetCurPtr(), len - sb.getPos());
  if (n > 0) {
    sb.SetPos(sb.GetPos() + n);
    SocketData::updateTimestamp(socket_, time(NULL));
  } else {
    smsc_log_warn(logger_, "Error: %s(%d)", strerror(errno), errno);
    return false;
  }
  if (sb.GetPos() >= len) {
    smsc_log_debug(logger_, "written to socket: len=%d, data=%s", n, sb.toString().c_str());
    outbuf_.Empty();
    action_ = READ_REQUEST;
    return false;
  }
  return true;
}

Socket* ConnectionContext::getSocket() {
  return socket_;
}

bool ConnectionContext::canFinalize() {
  MutexGuard mg(mutex_);
  return --tasksCount > 0 ? false : true;
}

}
}

