#include <errno.h>
#include <time.h>
#include "IOTasks.h"
#include "MmsContext.h"
#include "Managers.h"
#include "MmsParser.hpp"

#define READER_BUF_SIZE 8192
#define SOCKOP_TIMEOUT 10

namespace scag { namespace transport { namespace mms {

IOTask::IOTask(MmsManagerImpl& m, IOTaskManager& iom, int timeout) 
               : manager(m), iomanager(iom), connection_timeout(timeout) {
}

void IOTask::stop() {
  isStopping = true;
  MutexGuard g(sock_mon);
  sock_mon.notify();
}

bool IOTask::isTimedOut(Socket *s, time_t now) {
  return now - MmsContext::getTimestamp(s) > connection_timeout;
}

void IOTask::removeSocket(Socket* s) {
  {
    MutexGuard g(sock_mon);
    multiplexer.remove(s);
  }
  iomanager.removeContext(this, 1);
}

void IOTask::removeSocket(Multiplexer::SockArray &error) {
  if (!error.Count()) {
    return;
  }
  unsigned int nsub;
  {
    Socket* s;
    MutexGuard g(sock_mon);
    nsub = error.Count();
    while (error.Count()) {
      error.Pop(s);
      killSocket(s);
    }
    iomanager.removeContext(this, nsub);
  }
}

void IOTask::addSocket(Socket *s, bool connected) {
  MutexGuard g(sock_mon);
  MmsContext::updateTimestamp(s, time(NULL));
  MmsContext::setConnected(s, connected);
  waiting_add.Push(s);
  sock_mon.notify();
}

void IOTask::deleteSocket(Socket *s, char *buf, int how) {
  shutdown(s->getSocket(), how);
  while ((int)recv(s->getSocket(), buf, 4, 0) > 0);
  delete s;
}

void IOTask::killSocket(Socket *s) {
  MmsContext *cx = MmsContext::getContext(s);
  unsigned int flags = cx->flags;
  if (!(flags & NO_MULT_REM)) {
    multiplexer.remove(s);
  }
  if ((flags & DEL_SERVICE_SOCK) && cx->service) {
    delete cx->service;
    cx->service = NULL;
  }
  if ((flags & DEL_CLIENT_SOCK) && cx->client) {
    delete cx->client;
    cx->client = NULL;
  }
  if (flags & DEL_CONTEXT) {
    delete cx;
  } else {
    if (flags & STAT_RESP) {
      cx->action = PROCESS_STATUS_RESPONSE;
      manager.process(cx);
    }
    if (flags & FAKE_RESP) {
      cx->action = PROCESS_RESPONSE;
      manager.process(cx);
    }
  }
}

MmsReaderTask::MmsReaderTask(MmsManagerImpl &m, IOTaskManager &iom, int timeout):IOTask(m, iom, timeout) {
  logger = Logger::getInstance("mms.reader");
}

void MmsReaderTask::checkConnectionTimeout(Multiplexer::SockArray &error,
                                    Multiplexer::SockArray &incomplite) {
  error.Empty();
  incomplite.Empty();
  time_t now = time(NULL);
  for (int i = 0; i < multiplexer.Count(); ++i) {
    Socket *s = multiplexer.getSocket(i);
    if (isTimedOut(s,now)) {
      smsc_log_warn(logger, "socket %p timeout", s);
      MmsContext *cx = MmsContext::getContext(s);
      if (cx->action == READ_REQUEST) {
        if (cx->http_packet.isValid() && !cx->http_packet.isComplite()) {
          incomplite.Push(s);
        }
        //cx->setDestiny(http_status::REQUEST_TIMEOUT, FAKE_RESP);
      } else {
        cx->setDestiny(status::SERVICE_UNAVAILABLE, FAKE_RESP);
      }
      error.Push(s);
    }
  }
}

void MmsReaderTask::registerContext(MmsContext *cx) {
  //cx->flags = 0;
  //cx->result = 0;
  addSocket(cx->action == READ_RESPONSE ? cx->service : cx->client, true);
}

const char* MmsReaderTask::taskName() {
  return "mms.reader";
}

bool MmsReaderTask::createCommand(MmsContext *cx, Socket *s) {
  cx->http_packet.replaceSoapEnvelopeXmlnsValue();
  if (cx->action == READ_REQUEST) {
    smsc_log_debug(logger, "%p: %p request parsed", this, cx);
    return cx->createRequest();
  } else {
    smsc_log_debug(logger, "%p: %p response parsed", this, cx);
    return cx->createResponse(s);
  }
}

void MmsReaderTask::processIncomplitePackets(Multiplexer::SockArray &incomplite,
                                    Multiplexer::SockArray &error) {
  for (int i = 0; i < incomplite.Count(); ++i) {
    Socket *s = incomplite[i];
    MmsContext *cx = MmsContext::getContext(s);
    smsc_log_error(logger, "%p: %p process incomplite packet, socket %p", this, cx, s);
    if (createCommand(cx, s)) {
      removeSocket(s);
      manager.process(cx);
    } else {
      smsc_log_error(logger, "%p: %p create command error", this, cx);
      error.Push(s);
    }
  }
}

int MmsReaderTask::Execute() {
  Multiplexer::SockArray error;
  Multiplexer::SockArray ready;
  Multiplexer::SockArray incomplite;
  char buf[READER_BUF_SIZE];
  time_t now;
  smsc_log_debug(logger, "%p started", this);
  for (;;) {

    {
      MutexGuard g(sock_mon);
      while (!(socket_count || isStopping)) {
        smsc_log_debug(logger, "%p idle", this);
        sock_mon.wait();
        smsc_log_debug(logger, "%p notified", this);
      }
      if (isStopping) {
        break;
      }
      if (waiting_add.Count()) {
        Socket *s;
        waiting_add.Pop(s);
        multiplexer.add(s);
      }
    }

    checkConnectionTimeout(error, incomplite);

/***********************************************/

    processIncomplitePackets(incomplite, error);

/***********************************************/

    removeSocket(error);

    if (multiplexer.canRead(ready, error, SOCKOP_TIMEOUT) <= 0) {
      continue;
    }

    for (int i = 0; i < error.Count(); ++i) {
      Socket *s = error[i];
      MmsContext *cx = MmsContext::getContext(s);
      smsc_log_error(logger, "%p: %p failed", this, cx);
      cx->setDestiny(status::SERVICE_UNAVAILABLE, cx->action == READ_RESPONSE ?
                      FAKE_RESP | DEL_SERVICE_SOCK : DEL_CONTEXT);
    }
    now = time(NULL);
    for (int i = 0; i < ready.Count(); ++i) {
      Socket *s = ready[i];
      MmsContext *cx = MmsContext::getContext(s);
      smsc_log_debug(logger, "ready: context %p, socket %p", cx, s);
      int len = 0;
      int packet_len = 0;
      std::string packet;

      do {
        len = s->Read(buf, READER_BUF_SIZE);
        packet.append(buf, len);
        packet_len += len;
      } while (len == -1 && errno == EINTR);
      //smsc_log_debug(logger, "read packet size = %d packet:\n\'%s\'", packet_len, packet.c_str());

      if (len <= 0) {
        smsc_log_error(logger, "%p: %p, read error", this, cx);
        cx->setDestiny(status::SERVICE_ERROR, cx->action == READ_RESPONSE ?
            (FAKE_RESP | DEL_SERVICE_SOCK) : DEL_CONTEXT);
        error.Push(s);
        continue;
      }

      if (cx->http_packet.parse(packet.c_str(), packet_len)) {
        //smsc_log_debug(logger,"SoapEnvelope=\'%s\'", cx->http_packet.getSoapEnvelope());
        if (createCommand(cx, s)) {
          removeSocket(s);
          manager.process(cx);
        } else {
          smsc_log_error(logger, "%p: %p create command error", this, cx);
          error.Push(s);
        }
      } else {
        if (cx->http_packet.isComplite() || 
            (!cx->http_packet.isComplite() && (!cx->http_packet.isValid()))) {
          smsc_log_error(logger, "%p: %p parse error", this, cx);
          (cx->action == READ_RESPONSE) ?
            cx->setDestiny(status::SERVICE_UNAVAILABLE, FAKE_RESP | DEL_SERVICE_SOCK) :
            cx->setDestiny(http_status::BAD_REQUEST, FAKE_RESP);
          error.Push(s);
        } else {
          MmsContext::updateTimestamp(s, now);
          smsc_log_warn(logger, "%p: %p packet incomplite", this, cx);
        }
      }
    }
    removeSocket(error);
  }
  {
    Socket *s;
    MutexGuard g(sock_mon);
    while (multiplexer.Count()) {
      s = multiplexer.getSocket(0);
      MmsContext::getContext(s)->setDestiny(0, DEL_CONTEXT);
      killSocket(s);
    }
  }

  smsc_log_debug(logger, "%p quit", this);
  return 0;
}

MmsWriterTask::MmsWriterTask(MmsManagerImpl &m, IOTaskManager &iom, int timeout):IOTask(m, iom, timeout) {
  logger = Logger::getInstance("mms.writer");
}

void MmsWriterTask::registerContext(MmsContext *cx) {
  Socket *s;
  //cx->flags = 0;
  //cx->result = 0;
  cx->position = 0;
  if (cx->action == SEND_REQUEST) {
    s = cx->service = new Socket();
    MmsContext::setContext(s,cx);
  } else {
    s = cx->client;
  }
  addSocket(s, cx->action != SEND_REQUEST);
}

void MmsWriterTask::checkConnectionTimeout(Multiplexer::SockArray &error) {
  error.Empty();
  time_t now = time(NULL);
  for (int i = 0; i < multiplexer.Count(); ++i) {
    Socket *s = multiplexer.getSocket(i);
    if (isTimedOut(s,now)) {
      smsc_log_warn(logger, "socket %p timeout", s);
      MmsContext *cx = MmsContext::getContext(s);
      if (cx->action == SEND_REQUEST) {
        cx->setDestiny(status::SERVICE_UNAVAILABLE, FAKE_RESP | DEL_SERVICE_SOCK);
      } else {
        cx->setDestiny(http_status::INTERNAL_SERVER_ERROR, STAT_RESP | DEL_CLIENT_SOCK);
      }
      error.Push(s);
    }
  }
}

const char* MmsWriterTask::taskName() {
  return "mms.writer";
}

int MmsWriterTask::Execute() {
  Multiplexer::SockArray error;
  Multiplexer::SockArray ready;
  char buf[4];
  time_t now;
  smsc_log_debug(logger, "%p started", this);
  for (;;) {

    {
      MutexGuard g(sock_mon);
      while (!(socket_count || isStopping)) {
        smsc_log_debug(logger, "%p idle", this);
        sock_mon.wait();
        smsc_log_debug(logger, "%p notified", this);
      }
      if (isStopping) {
        break;
      }
      if (waiting_add.Count()) {
        Socket *s;
        waiting_add.Pop(s);
        MmsContext::getConnected(s) ? multiplexer.add(s) : waiting_connect.Push(s);
      }
    }

    checkConnectionTimeout(error);

    while (waiting_connect.Count()) {
      Socket *s;
      waiting_connect.Pop(s);
      MmsContext *cx = MmsContext::getContext(s);
      smsc_log_debug(logger, "%p: %p, connecting %s:%d", this, cx, 
                     cx->getRequest().getDestHost().c_str(), cx->getRequest().getDestPort());
      if (s->Init(cx->getRequest().getDestHost().c_str(), cx->getRequest().getDestPort()
                  , SOCKOP_TIMEOUT) || s->Connect(true)) {
        smsc_log_error(logger, "%p: %p, can't connect", this, cx);
        cx->setDestiny(status::SERVICE_UNAVAILABLE, 
                       FAKE_RESP | DEL_SERVICE_SOCK | NO_MULT_REM); //503
        error.Push(s);
      } else {
        smsc_log_debug(logger, "%p: %p, socket %p connect", this, cx, s);
        multiplexer.add(s);
      }
    }

    removeSocket(error);

    if (multiplexer.canWrite(ready, error, SOCKOP_TIMEOUT) > 0) {
      for (int i = 0; i < error.Count(); ++i) {
        Socket *s = error[i];
        MmsContext *cx = MmsContext::getContext(s);
        smsc_log_error(logger, "%p: %p faild", this, cx);
        (cx->action == SEND_REQUEST) ?
          cx->setDestiny(status::SERVICE_UNAVAILABLE, FAKE_RESP | DEL_SERVICE_SOCK) :
          cx->setDestiny(http_status::INTERNAL_SERVER_ERROR, STAT_RESP | DEL_CLIENT_SOCK); 
      }
    }
    now = time(NULL);
    for (int i = 0; i < ready.Count(); ++i) {
      Socket *s = ready[i];
      MmsContext *cx = MmsContext::getContext(s);
      const char* data = cx->http_packet.getPacket() + cx->position;
      //smsc_log_debug(logger,"write data:\'%s\'", data);
      unsigned int size = cx->http_packet.getPacketSize() - cx->position;
      if (size) {
        int writen_size = 0;

        do {
          writen_size = s->Write(data, size);
        } while (writen_size == -1 && errno == EINTR);

        smsc_log_debug(logger, "writen %d bytes", writen_size);

        if (writen_size > 0) {
          cx->position += writen_size;
          MmsContext::updateTimestamp(s, now);
        } else {
          smsc_log_error(logger, "%p: %p, write error", this, cx);
          (cx->action == SEND_REQUEST) ?
            cx->setDestiny(status::SERVICE_UNAVAILABLE, FAKE_RESP | DEL_SERVICE_SOCK) :
            cx->setDestiny(http_status::INTERNAL_SERVER_ERROR, 
                           STAT_RESP | DEL_CLIENT_SOCK);
          error.Push(s);
        }
      }

      if (cx->position >= cx->http_packet.getPacketSize()) {
        removeSocket(s);
        if (cx->action == SEND_REQUEST) {
          smsc_log_info(logger, "%p: %p, request sent", this, cx);
          cx->action = READ_RESPONSE;
          cx->result = 0;
          cx->http_packet.clear();
          manager.readerProcess(cx);
        } else {
          smsc_log_info(logger, "%p: %p, response sent", this, cx);
          deleteSocket(s, buf, SHUT_WR);
          cx->action = PROCESS_STATUS_RESPONSE;
          cx->result = 0;
          cx->client = NULL;
          manager.process(cx);
        }
      }

      removeSocket(error);

    }
  }

  {
    Socket *s;
    MutexGuard g(sock_mon);
    while (multiplexer.Count()) {
      s = multiplexer.getSocket(0);
      MmsContext::getContext(s)->setDestiny(0, DEL_CONTEXT);
      killSocket(s);
    }
    while (waiting_connect.Count()) {
      waiting_connect.Pop(s);
      MmsContext::getContext(s)->setDestiny(0, DEL_CONTEXT);
      killSocket(s);
    }
  }

  smsc_log_debug(logger, "%p quit", this);

  return 0;
}

}//mms
}//transport
}//scag 


// 
// 
  /*
        if (cx->action == READ_REQUEST) {
          smsc_log_debug(logger, "%p: %p request parsed", this, cx);
          if (cx->createRequest()) {
            manager.process(cx);
          } else {
            smsc_log_error(logger, "%p: %p create request command error", this, cx);
            error.Push(s);
          }
        } else {
          smsc_log_debug(logger, "%p: %p response parsed", this, cx);
          if (cx->createResponse(s)) {
            manager.process(cx);
          } else {
            smsc_log_error(logger, "%p: %p create response command error", this, cx);
            error.Push(s);
          }
        }*/
    //????????????????????????????????????????//
    /*if (multiplexer.canWrite(ready, error, SOCKOP_TIMEOUT) <= 0) {
      continue;
    }
    for (unsigned int i = 0; i < (unsigned int)incomplite.Count(); ++i) {
      Socket *s = incomplite[i];
      MmsContext *cx = MmsContext::getContext(s);
      for (unsigned int j = 0; j < (unsigned int)ready.Count(); ++j) {
        if (s == ready[i] && cx->action == READ_REQUEST) {
          smsc_log_warn(logger, "%p: p socket %p ready for write", this, cx, s);
          if (cx->createRequest()) {
            manager.process(cx);
          } else {
            smsc_log_error(logger, "%p: %p create request command error", this, cx);
          }
          removeSocket(s);
          break;
        }
      }
    }*/
    //????????????????????????????????????????//

