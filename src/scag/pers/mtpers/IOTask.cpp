#include "IOTask.h"
#include "IOTaskManager.h"

namespace scag { namespace mtpers { 

const size_t MAX_PACKET_SIZE = 100000;
const int SOCKOP_TIMEOUT = 1000;

using smsc::core::synchronization::MutexGuard;

IOTask::IOTask(IOTaskManager& iomanager, uint16_t connectionTimeout):iomanager_(iomanager),
               connectionTimeout_(connectionTimeout)
{
  logger = Logger::getInstance("iotask");
}

bool IOTask::idle() {
  return !(itemsCount_ && (waitingAdd_.Count() || multiplexer_.count())) && !isStopping;
}

int IOTask::Execute() {  
  Multiplexer::SockArray read;
  Multiplexer::SockArray write;
  Multiplexer::SockArray error;

  smsc_log_debug(logger, "%p started", this);

  for (;;) {
    {
      MutexGuard g(socketMonitor_);

      while (idle()) {
        smsc_log_debug(logger, "%p idle", this);
        socketMonitor_.wait();         
        smsc_log_debug(logger, "%p notified", this);
      }
      if (isStopping) {
        break;
      }

      while(waitingAdd_.Count()) {
        Socket *s;
        waitingAdd_.Pop(s);
        addSocketToMultiplexer(s);
      }
    }

    checkConnectionTimeout(error);

    removeSocket(error);

    if (multiplexer_.canReadWrite(read, write, error, connectionTimeout_ * 1000)) {
      for (int i = 0; i < write.Count(); ++i) {
        Socket* s = write[i];
        if (!processWriteSocket(s)) {
          error.Push(s);
        }
      }
      for (int i = 0; i < read.Count(); ++i) {
        Socket* s = read[i];
        if (!processReadSocket(read[i])) {
          error.Push(s);
        }
      }
    }
    removeSocket(error);
  }
  {
    Socket* s;
    ConnectionContext* cx;
    MutexGuard g(socketMonitor_);

    while (multiplexer_.count()) {
      s = multiplexer_.get(0);
      cx = SocketData::getContext(s);
      multiplexer_._remove(0);
      delete cx;
    }
  }

  smsc_log_debug(logger, "%p quit", this);

  return 0;
}

const char* IOTask::taskName() {
  return "IOTask";
}

uint32_t IOTask::getSocketsCount() const {
  return itemsCount_;
}

void IOTask::addSocket(Socket* s) {
  MutexGuard g(socketMonitor_);

  SocketData::updateTimestamp(s, time(NULL));
  waitingAdd_.Push(s);
   
  socketMonitor_.notify();
}

void IOTask::registerContext(ConnectionContext* cx) {
  addSocket(cx->socket);
  cx->iotask = this;
}

void IOTask::addSocketToMultiplexer(Socket* s) {
  SocketState state = SocketData::getSocketState(s);
  switch (state) {
  case READ_SOCKET: 
    multiplexer_.addR(s);
    smsc_log_debug(logger, "%p: add READ socket %p", this, s);
    break;
  case WRITE_SOCKET:
    multiplexer_.addW(s);
    smsc_log_debug(logger, "%p: add WRITE socket %p", this, s);
    break;
  case READWRITE_SOCKET: 
    multiplexer_.addRW(s);
    smsc_log_debug(logger, "%p: add RW socket %p", this, s);
    break;
  };
}

void IOTask::changeSocketState(Socket *s) {
  MutexGuard g(socketMonitor_);
  smsc_log_debug(logger, "%p: change state socket %p", this, s);
  addSocketToMultiplexer(s);
}

void IOTask::checkConnectionTimeout(Multiplexer::SockArray& error) {
  error.Empty();
  time_t now = time(NULL);
  for (int i = 0; i < multiplexer_.count(); i++) {
    Socket *s =  multiplexer_.get(i);
    if (isTimedOut(s, now)) {
      smsc_log_debug(logger, "%p: socket %p timeout", s);
      error.Push(s);
    }
  }
}

inline bool IOTask::isTimedOut(Socket* s, time_t now) {
  return now - SocketData::getTimestamp(s) >= connectionTimeout_;
}

void IOTask::removeSocket(Multiplexer::SockArray &error) {
  if (!error.Count()) {
    return;
  }
  Socket* s;
  uint16_t contextsCount = error.Count();
  {
    MutexGuard g(socketMonitor_);
    while (error.Count()) {
      error.Pop(s);
      smsc_log_warn(logger, "%p: socket %p failed", this, s);
      disconnectSocket(s);
    }
  }
  iomanager_.removeContext(this, contextsCount);
}

void IOTask::disconnectSocket(Socket *s) {
  ConnectionContext* cx = SocketData::getContext(s);
  smsc_log_debug(logger, "%p: context %p socket %p disconnected", this, cx, s);
  multiplexer_.remove(s);
  s->Close();
  delete cx;
}

inline void IOTask::removeSocket(Socket *s) {
  {
    MutexGuard g(socketMonitor_);
    disconnectSocket(s);
  }
  iomanager_.removeContext(this);
}

void IOTask::removeSocketFromMultiplexer(Socket* s) {
  MutexGuard g(socketMonitor_);
  multiplexer_.remove(s);
}


bool IOTask::processReadSocket(Socket* s) {
  ConnectionContext* cx = SocketData::getContext(s);
  if (cx->action != READ_REQUEST) {
    smsc_log_debug(logger, "%p: cx %p action=%d", this, cx, cx->action);
    return false;
  }
  SerialBuffer& sb = cx->inbuf;

  if(sb.GetSize() < PACKET_LENGTH_SIZE) {
    int n = s->Read(readBuf_, PACKET_LENGTH_SIZE - sb.GetSize());
    smsc_log_debug(logger, "%p: read(len) %u bytes from %p", this, n, s);
    if (n <= 0) {
      if (n) smsc_log_debug(logger, "%p: read error: %s(%d)", this, strerror(errno), errno);
      return false;
    }
    sb.Append(readBuf_, n);
    if (sb.GetSize() < PACKET_LENGTH_SIZE) {
      return true;
    }
    n = sb.GetPos();
    sb.SetPos(0);
    cx->packetLen = sb.ReadInt32();
    smsc_log_debug(logger, "%d bytes will be read from %x", cx->packetLen, s);
    if(cx->packetLen > MAX_PACKET_SIZE) {
      smsc_log_warn(logger, "Too big packet from client");
      return false;
    }
    sb.SetPos(n);
  } 
  int n = cx->packetLen - sb.GetSize();
  n = s->Read(readBuf_, n > READ_BUF_SIZE ? READ_BUF_SIZE : n);

  smsc_log_debug(logger, "read %u bytes from %p", n, s);

  if (n > 0) {
    sb.Append(readBuf_, n);
  } else if(errno != EWOULDBLOCK) {
    if (n) smsc_log_debug(logger, "read error: %s(%d)", strerror(errno), errno);
    return false;
  }
  if (sb.GetSize() < cx->packetLen) {
    return true;
  }
  smsc_log_debug(logger, "read from socket:%p len=%d, data=%s", s, sb.length(), sb.toString().c_str());
  cx->inbuf.SetPos(PACKET_LENGTH_SIZE);
  if (!cx->parsePacket()) {
    changeSocketState(s);
    return true;
  }
  if (iomanager_.storageProcess(cx)) {
    removeSocketFromMultiplexer(s);
    //SocketData::setSocketState(s, READWRITE_SOCKET);
  } else {
    cx->createFakeResponse(scag::pers::RESPONSE_ERROR);
    changeSocketState(s);
  }
  return true;
}

bool IOTask::processWriteSocket(Socket* s) {
  ConnectionContext* cx = SocketData::getContext(s);
  if (cx->action != SEND_RESPONSE) {
    return true;
  }
  SerialBuffer& sb = cx->outbuf;
  uint32_t len = sb.GetSize();
  if (len == 0) {
    return true;
  }
  smsc_log_debug(logger, "write %u bytes to %p, GetCurPtr: %x, GetPos: %d data=%s",
                  len, s, sb.GetCurPtr(), sb.GetPos(), sb.toString().c_str());

  int n = s->Write(sb.GetCurPtr(), len - sb.GetPos());
  if (n > 0) {
    sb.SetPos(sb.GetPos() + n);
    SocketData::updateTimestamp(s, time(NULL));
  } else {
    smsc_log_debug(logger, "Error: %s(%d)", strerror(errno), errno);
    return false;
  }
  if (sb.GetPos() >= len) {
    smsc_log_debug(logger, "written to socket: len=%d, data=%s", n, sb.toString().c_str());
    cx->clearBuffers();
    SocketData::setSocketState(s, READ_SOCKET);
    cx->action = READ_REQUEST;
    changeSocketState(s);
  }
  return true;
}

void IOTask::stop() {
  isStopping = true;

  MutexGuard g(socketMonitor_);
  socketMonitor_.notify();
}

}//mtpers
}//scag

