#include "ConnectionContext.h"
#include "WriterTaskManager.h"
#include "ReaderTaskManager.h"

#include "scag/util/RelockMutexGuard.h"

namespace scag2 {
namespace pvss  { 

using smsc::core::synchronization::MutexGuard;
using scag::util::RelockMutexGuard;

const size_t MAX_PACKET_SIZE = 102400;

ConnectionContext::ConnectionContext(Socket* sock, WriterTaskManager& writerManager, core::server::ServerCore& server, bool perfCounterOn)
                   : action_(READ_REQUEST), packetLen_(0), async_(false), pvssServer_(server), socket_(sock),  
                    writerManager_(writerManager), perfCounter_(perfCounterOn), tasksCount_(1), packetsCount_(0)
{
  //if (socket_) {
    //SocketData::setContext(socket_, this);
  //}
  debuglogger_ = Logger::getInstance("ctx");
  getPeerIp();
} 

ConnectionContext::~ConnectionContext() {
  if (socket_) {
    delete socket_;
  }
}

void ConnectionContext::getPeerIp() {
  if (!socket_) {
    return;
  }
  char peerNameBuf[32];
  socket_->GetPeer(peerNameBuf);
  peerIp_ = peerNameBuf;
  string::size_type pos = peerIp_.find_first_of(':');
  if (pos != string::npos) {
    peerIp_.erase(pos);
  }
  //smsc_log_info(logger_, "Connection accepted from %s", peerIp_.c_str());
}

bool ConnectionContext::sendResponseData(const char* data, uint32_t dataSize) {
  {
    MutexGuard mg(mutex_);
    if (!data || dataSize == 0) {
      action_ = READ_REQUEST;
      --packetsCount_;
      return false;
    }
    action_ = SEND_RESPONSE;
    bool inprocess = outbuf_.GetSize() == 0 ? false : true;
    writeData(data, dataSize);
    perfCounter_.incProcessed();
    --packetsCount_;
    if (tasksCount_ >= 2) {
      smsc_log_debug(logger_, "cx:%p socket %p error tasksCount=%d", this, socket_, tasksCount_);
    }
    if (inprocess) {
      smsc_log_debug(logger_, "cx:%p socket %p already in multiplexer", this, socket_);
      return true;
    }
    ++tasksCount_;
  }
  if (!writerManager_.process(this)) {
    //TODO: error, response must be processed
    MutexGuard mg(mutex_);
    --tasksCount_;
    return false;
  }
  return true;
}

void ConnectionContext::writeData(const char* data, uint32_t size) {
  if (!data || size == 0) {
    return;
  }
  uint32_t pos = outbuf_.getPos();
  uint32_t bufSize = outbuf_.GetSize();
  outbuf_.SetPos(bufSize);
  outbuf_.WriteInt32(size + PACKET_LENGTH_SIZE);
  outbuf_.Append(data, size);
  outbuf_.SetPos(pos);
}

bool ConnectionContext::readData(const time_t& now) {
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
    if (packetLen_ > MAX_PACKET_SIZE) {
      smsc_log_warn(logger_, "Too big packet from client: %d bytes. Max packet size: %d bytes", packetLen_, MAX_PACKET_SIZE);
      return false;
    }
    inbuf_.SetPos(n);
  } 
  int n = packetLen_ - inbuf_.GetSize();
  n = socket_->Read(readBuf_, n > READ_BUF_SIZE ? READ_BUF_SIZE : n);

  smsc_log_debug(logger_, "read %u bytes from %p", n, socket_);

  if (n > 0) {
    SocketData::updateTimestamp(socket_, now);
    inbuf_.Append(readBuf_, n);
  } else if (errno != EWOULDBLOCK) {
    if (n) smsc_log_warn(logger_, "read error: %s(%d)", strerror(errno), errno);
    return false;
  }
  perfCounter_.incAccepted();
  {
    MutexGuard mg(mutex_);
    ++packetsCount_;
    action_ = PROCESS_REQUEST;
  }
  smsc_log_debug(logger_, "read from socket:%p len=%d, data=%s", socket_, inbuf_.length(), inbuf_.toString().c_str());
  inbuf_.SetPos(PACKET_LENGTH_SIZE);
  return true;
}

/*
void ConnectionContext::flushLogs() {
  if (dbLogs_.empty()) {
    return;
  }
  for (vector<DbLog>::iterator i = dbLogs_.begin(); i != dbLogs_.end(); ++i) {
    (*i).flush();
  }
  dbLogs_.clear();
}
*/

bool ConnectionContext::processWriteSocket(const time_t& now) {
  RelockMutexGuard mg(mutex_);
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
  SocketData::updateTimestamp(socket_, now);
  //flushLogs();
  if (n > 0) {
    sb.SetPos(sb.GetPos() + n);
  } else {
    outbuf_.Empty();
    mg.Unlock();
    smsc_log_warn(logger_, "Error: %s(%d)", strerror(errno), errno);
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

Socket* ConnectionContext::getSocket() const {
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

