#include "ConnectionContext.h"
#include "WriterTaskManager.h"
#include "ReaderTaskManager.h"

namespace scag { namespace mtpers { 

using smsc::core::synchronization::MutexGuard;
const size_t MAX_PACKET_SIZE = 100000;

ConnectionContext::ConnectionContext(Socket* sock, WriterTaskManager& writerManager, ReaderTaskManager& readerManager, bool perfCounterOn)
                   :socket_(sock), action_(READ_REQUEST), packetLen_(0), writerManager_(writerManager), 
                    readerManager_(readerManager), tasksCount_(1), async_(false), sequenceNumber_(0), packetsCount_(0), perfCounter_(perfCounterOn)
{
  if (socket_) {
    SocketData::setContext(socket_, this);
  }
  logger_ = Logger::getInstance("context");
  debuglogger_ = Logger::getInstance("ctx");
} 

ConnectionContext::~ConnectionContext() {
  if (socket_) {
    delete socket_;
  }
}

void ConnectionContext::createFakeResponse(PersServerResponseType response) {
  smsc_log_debug(logger_, "%p: Create Fake response %d", this, response);
  fakeResp_.Empty();
  if (async_) {
    fakeResp_.WriteInt32(sequenceNumber_);
  }
  fakeResp_.WriteInt8(static_cast<uint8_t>(response));
}

bool ConnectionContext::notSupport(PersCmd cmd) {
  return (cmd > scag::pers::util::PC_MTBATCH || cmd == scag::pers::util::PC_BATCH
          || cmd == scag::pers::util::PC_TRANSACT_BATCH || cmd == scag::pers::util::PC_UNKNOWN) ? true : false;
}

PersPacket* ConnectionContext::parsePacket() {
  PersServerResponseType response = scag::pers::util::RESPONSE_BAD_REQUEST;
  try {
    sequenceNumber_ = 0;
    if (async_) {
      sequenceNumber_ = inbuf_.ReadInt32();
    }
    smsc_log_debug(logger_, "sequence number %d", sequenceNumber_);
    PersCmd cmd = (PersCmd)inbuf_.ReadInt8();
    if (cmd == scag::pers::util::PC_BIND_ASYNCH) {
      smsc_log_debug(logger_, "Asynch Bind received");
      if (async_) {
        createFakeResponse(scag::pers::util::RESPONSE_ERROR);
      } else {
        createFakeResponse(scag::pers::util::RESPONSE_OK);
        async_ = true;
      }
      return NULL;
    }
    smsc_log_debug(logger_, "Command %d received", cmd);
    if (cmd == scag::pers::util::PC_PING) {
      smsc_log_debug(logger_, "Ping received");
      createFakeResponse(scag::pers::util::RESPONSE_OK);
      return NULL;
    }
    if (notSupport(cmd)) {
      createFakeResponse(scag::pers::util::RESPONSE_NOTSUPPORT);
      return NULL;
    }
    std::auto_ptr<PersPacket> packet;
    if (cmd == scag::pers::util::PC_MTBATCH) {
      smsc_log_debug(logger_, "Batch received");
      packet.reset( new BatchPacket(this, async_, sequenceNumber_) );
    } else {
      packet.reset( new CommandPacket(this, cmd, async_, sequenceNumber_) );
    }
    packet->deserialize(inbuf_);
    return packet.release();
  } catch(const SerialBufferOutOfBounds& e) {
    smsc_log_warn(logger_, "SerialBufferOutOfBounds Bad data in buffer received len=%d, curpos=%d, data=%s",
                   inbuf_.length(), inbuf_.GetPos(), inbuf_.toString().c_str());
  } catch(const std::runtime_error& e) {
    smsc_log_warn(logger_, "std::runtime_error: Error profile key: %s. received buffer len=%d, data=%s",
                   e.what(), inbuf_.length(), inbuf_.toString().c_str());
  } catch(const PersCommandNotSupport& e) {
    smsc_log_warn(logger_, "std::runtime_error: Not support command. received buffer len=%d, data=%s",
                   inbuf_.length(), inbuf_.toString().c_str());
    response = scag::pers::util::RESPONSE_NOTSUPPORT;
  }
  createFakeResponse(response);
  return NULL;
}

void ConnectionContext::sendFakeResponse() {
  if (writerManager_.process(this)) {
    ++tasksCount_;
  }
}

void ConnectionContext::sendResponse(const char* data, uint32_t dataSize) {
  perfCounter_.incProcessed();
  {
    MutexGuard mg(mutex_);
    action_ = SEND_RESPONSE;
    bool inprocess = outbuf_.GetSize() == 0 ? false : true;
    writeData(data, dataSize);
    --packetsCount_;
    if (tasksCount_ >= 2) {
      smsc_log_debug(logger_, "cx:%p socket %p error tasksCounr=%d", this, socket_, tasksCount_);
    }
    if (inprocess) {
      smsc_log_debug(logger_, "cx:%p socket %p already in multiplexer", this, socket_);
      return;
    }
    ++tasksCount_;
  }
  if (!writerManager_.process(this)) {
    //TODO: error, response must be processed
    MutexGuard mg(mutex_);
    --tasksCount_;
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


bool ConnectionContext::processReadSocket(const time_t& now) {
  if (!async_) {
    MutexGuard mg(mutex_);
    if (action_ != READ_REQUEST) {
      smsc_log_warn(logger_, "cx:%p socket %p error action=%d, must be READ_REQUEST", this, socket_, action_);
      return false;
    }
  }

  if(inbuf_.GetSize() < PACKET_LENGTH_SIZE) {
    int n = socket_->Read(readBuf_, PACKET_LENGTH_SIZE - inbuf_.GetSize());
    smsc_log_debug(logger_, "cx:%p read(len) %u bytes from %p", this, n, socket_);
    if (n <= 0) {
      if (n) smsc_log_warn(logger_, "%p: read error: %s(%d)", this, strerror(errno), errno);
      return false;
    }
    inbuf_.Append(readBuf_, n);
    if (inbuf_.GetSize() < PACKET_LENGTH_SIZE) {
      SocketData::updateTimestamp(socket_, now);
      return true;
    }
    n = inbuf_.getPos();
    inbuf_.SetPos(0);
    packetLen_ = inbuf_.ReadInt32();
    smsc_log_debug(logger_, "%d bytes will be read from %p", packetLen_, socket_);
    if(packetLen_ > MAX_PACKET_SIZE) {
      smsc_log_warn(logger_, "Too big packet from client");
      return false;
    }
    inbuf_.SetPos(n);
  } 
  int n = packetLen_ - inbuf_.GetSize();
  n = socket_->Read(readBuf_, n > READ_BUF_SIZE ? READ_BUF_SIZE : n);

  smsc_log_debug(logger_, "read %u bytes from %p", n, socket_);

  if (n > 0) {
    SocketData::updateTimestamp(socket_, time(NULL));
    inbuf_.Append(readBuf_, n);
  } else if(errno != EWOULDBLOCK) {
    if (n) smsc_log_warn(logger_, "read error: %s(%d)", strerror(errno), errno);
    return false;
  }
  if (inbuf_.GetSize() < packetLen_) {
    return true;
  }
  perfCounter_.incAccepted();
  MutexGuard mg(mutex_); {
    ++packetsCount_;
    action_ = PROCESS_REQUEST;
  }
  smsc_log_debug(logger_, "read from socket:%p len=%d, data=%s", socket_, inbuf_.length(), inbuf_.toString().c_str());
  inbuf_.SetPos(PACKET_LENGTH_SIZE);
  PersPacket* packet = parsePacket();
  smsc_log_info(debuglogger_, "process seq.number:%d packet:%p key:%d/'%s' packet_size:%d socket:%p",
                             packet->getSequenceNumber(), packet, packet->intKey, packet->address.toString().c_str(), packetLen_, socket_);
  inbuf_.Empty();
  packetLen_ = 0;

  if (!packet) {
    sendResponse(fakeResp_.c_ptr(), fakeResp_.GetSize());
    return true;
  }
  if (!readerManager_.processPacket(packet)) {
    createFakeResponse(scag::pers::util::RESPONSE_ERROR);
    sendResponse(fakeResp_.c_ptr(), fakeResp_.GetSize());
  }
  return true;
}

bool ConnectionContext::processWriteSocket() {
  MutexGuard mg(mutex_);
  if (!async_ && action_ != SEND_RESPONSE) {
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
  } else {
    smsc_log_warn(logger_, "Error: %s(%d)", strerror(errno), errno);
    outbuf_.Empty();
    return false;
  }
  if (sb.GetPos() >= len) {
    smsc_log_debug(logger_, "written %u bytes to %p data=%s", n, socket_, sb.toString().c_str());
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
  return (--tasksCount_ > 0 || packetsCount_ > 0) ? false : true;
}

bool ConnectionContext::canDelete() {
  MutexGuard mg(mutex_);
  return --tasksCount_ > 0 ? false : true;
}


}
}

