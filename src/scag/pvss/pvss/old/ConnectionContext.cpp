#include "ConnectionContext.h"
// #include "WriterTaskManager.h"
#include "IOTask.h"

#include "scag/util/RelockMutexGuard.h"

namespace scag2 {
namespace pvss  { 

using smsc::core::synchronization::MutexGuard;
using scag::util::RelockMutexGuard;
using scag2::util::storage::SerialBuffer;
using smsc::core::network::Socket;
using smsc::logger::Logger;

const size_t MAX_PACKET_SIZE = 102400;

ConnectionContext::ConnectionContext( Socket* sock,
                                      core::server::ServerCore& server,
                                      bool perfCounterOn ) :
Connection(sock),
action_(READ_REQUEST), packetLen_(0),
async_(false),
pvssServer_(server),
perfCounter_(perfCounterOn),
packetsCount_(0),
writer_(0)
{
} 

ConnectionContext::~ConnectionContext() 
{
}

/*
void ConnectionContext::getPeerIp() 
{
  if (!getSocket()) {
    return;
  }
  char peerNameBuf[32];
  getSocket()->GetPeer(peerNameBuf);
  peerIp_ = peerNameBuf;
  peerName_ = peerIp_;
    std::string::size_type pos = peerIp_.find_first_of(':');
    if (pos != std::string::npos) {
        peerIp_.erase(pos);
    }
}
 */


bool ConnectionContext::sendResponseData(const char* data, uint32_t dataSize) {
  {
    MutexGuard mg(mutex_);
    if (!data || dataSize == 0) {
      action_ = READ_REQUEST;
      --packetsCount_;
      return false;
    }
    action_ = SEND_RESPONSE;
    // bool inprocess = outbuf_.GetSize() == 0 ? false : true;
    writeData(data, dataSize);
    perfCounter_.incProcessed();
    --packetsCount_;
    /*
    if (tasksCount_ >= 2) {
      smsc_log_debug(log_, "cx:%p socket %p error tasksCount=%d", this, getSocket(), tasksCount_);
    }
     */
    /*
    if (inprocess) {
      smsc_log_debug(log_, "cx:%p socket %p already in multiplexer", this, getSocket());
      return true;
    }
    ++tasksCount_;
     */
  }
    /*
  if (!writerManager_.registerConnection( this ) ) {
    //TODO: error, response must be processed
    MutexGuard mg(mutex_);
    --tasksCount_;
    return false;
  }
     */
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
    kickWriter();
}


bool ConnectionContext::readData(const time_t& now) {
  if(inbuf_.GetSize() < PACKET_LENGTH_SIZE) {
    int n = getSocket()->Read(readBuf_, PACKET_LENGTH_SIZE - inbuf_.GetSize());
    smsc_log_debug(log_, "cx:%p read(len) %u bytes from %p", this, n, getSocket());
    if (n <= 0) {
      if (n) {
          smsc_log_warn(log_, "%p: read error: %s(%d)", this, strerror(errno), errno);
      }
      kickWriter();
      return false;
    }
    inbuf_.Append(readBuf_, n);
    if (inbuf_.GetSize() < PACKET_LENGTH_SIZE) {
      SocketData::updateTimestamp(getSocket(), now);
      return true;
    }
    n = inbuf_.getPos();
    inbuf_.SetPos(0);
    packetLen_ = inbuf_.ReadInt32();
    smsc_log_debug(log_, "%d bytes will be read from %p", packetLen_, getSocket());
    if (packetLen_ > MAX_PACKET_SIZE) {
      smsc_log_warn(log_, "Too big packet from client: %d bytes. Max packet size: %d bytes", packetLen_, MAX_PACKET_SIZE);
      return false;
    }
    inbuf_.SetPos(n);
  } 
  int n = packetLen_ - inbuf_.GetSize();
  n = getSocket()->Read(readBuf_, n > READ_BUF_SIZE ? READ_BUF_SIZE : n);

  smsc_log_debug(log_, "read %u bytes from %p", n, getSocket());

  if (n > 0) {
    SocketData::updateTimestamp(getSocket(), now);
    inbuf_.Append(readBuf_, n);
    if (inbuf_.GetSize() < packetLen_) {
      return true;
    }
  } else if (errno != EWOULDBLOCK) {
    if (n) {
        smsc_log_warn(log_, "read error: %s(%d)", strerror(errno), errno);
    }
    return false;
  }
  perfCounter_.incAccepted();
  {
    MutexGuard mg(mutex_);
    ++packetsCount_;
    action_ = PROCESS_REQUEST;
  }
  smsc_log_debug(log_, "read from socket:%p len=%d, data=%s", getSocket(), inbuf_.length(), inbuf_.toString().c_str());
  inbuf_.SetPos(PACKET_LENGTH_SIZE);
  return true;
}

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
  smsc_log_debug(log_, "write %u bytes to %p, GetCurPtr: %x, GetPos: %d data=%s",
                  len, getSocket(), sb.GetCurPtr(), sb.GetPos(), sb.toString().c_str());

  int n = getSocket()->Write(sb.GetCurPtr(), len - sb.getPos());
  SocketData::updateTimestamp(getSocket(), now);
  //flushLogs();
  if (n > 0) {
    sb.SetPos(sb.GetPos() + n);
    kickWriter();
  } else {
    outbuf_.Empty();
    mg.Unlock();
    smsc_log_warn(log_, "Error: %s(%d)", strerror(errno), errno);
    return false;
  }
  if (sb.GetPos() >= len) {
    smsc_log_debug(log_, "written %u bytes to %p data=%s", n, getSocket(), sb.toString().c_str());
    outbuf_.Empty();
    action_ = READ_REQUEST;
  }
  return true;
}


void ConnectionContext::unregisterFromCore()
{
    pvssServer_.closeChannel(*this);
    kickWriter();
}


void ConnectionContext::setWriter( MTPersWriter* wr )
{
    writer_ = wr;
}


void ConnectionContext::kickWriter()
{
    MTPersWriter* wr = writer_;
    if (wr) {
        wr->packetIsReady();
    }
}

}
}

