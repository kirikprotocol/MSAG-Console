#include "PersServer.h"

namespace scag2 {
namespace pvss  {

using smsc::util::Exception;
using smsc::core::synchronization::MutexGuard;

PersServer::PersServer(core::server::ServerCore& server, ReaderTaskManager& readers, WriterTaskManager& writers, PersProtocol& protocol, bool perfCounterOn, int perfCounterPeriod)
                       :pvssServer_(server), readers_(readers), writers_(writers), protocol_(protocol), perfCounterOn_(perfCounterOn),
                        perfCounterPeriod_(perfCounterPeriod), isStopping_(true) 
{
  perfCounterPeriod_ = perfCounterOn_ ? perfCounterPeriod : 0;
  readers_.init();
  writers_.init();
}

int PersServer::Execute()
{
  Socket *clientSocket;

  smsc_log_info(logger, "pers server started");

  time_t lastTime = time(NULL);

  Performance averPerf;
  while (!isStopped()) {

    clientSocket = masterSocket_.Accept(perfCounterPeriod_);

    if (isStopped()) {
      break;
    }

    if (!clientSocket) {
      time_t now = time(NULL);
      if (!perfCounterOn_ || now - lastTime < perfCounterPeriod_) {
        smsc_log_error(logger, "failed to accept, error: %s", strerror(errno));
        continue;
      }
      Performance perf = readers_.getPerformance();
      averPerf.inc(perf);
      if ( perf.accepted != 0 || perf.processed != 0 ) {
          smsc_log_info(logger, "current performance %d:%d req/sec. sync connections: %d", perf.accepted, perf.processed, perf.connections);
      }
      lastTime = now;
      continue;
    }

    if (perfCounterOn_) {
      time_t now = time(NULL);
      if (now - lastTime >= perfCounterPeriod_) {
        Performance perf = readers_.getPerformance();
        averPerf.inc(perf);
        if ( perf.accepted != 0 || perf.processed != 0 ) {
            smsc_log_info(logger, "current performance %d:%d req/sec. sync connections: %d", perf.accepted, perf.processed, perf.connections);
        }
        lastTime = now;
      }
    }

    //ConnectionContext *cx = new ConnectionContext(clientSocket, writers_, readers_, perfCounterOn_);
    ConnectionContext *cx = new SyncContext(clientSocket, writers_, pvssServer_, protocol_, perfCounterOn_);
    smsc_log_debug(logger, "Client connected socket:%p context:%p", clientSocket, cx);
    if (!readers_.process(cx)) {
      clientSocket->Write("SB", 2);
      smsc_log_warn(logger, "Server Busy sent. Disconnected");
      delete cx;
    }
    //TODO: check license?
  }
  smsc_log_info(logger, "averege performance %d:%d req/sec", averPerf.getAveregeAccepted(), averPerf.getAveregeProcessed());

  if (clientSocket) {
    delete clientSocket;
  }

  smsc_log_debug(logger, "quit");

  return 1;
}

const char* PersServer::taskName()
{
  return "MTPersServer";
}

void PersServer::stop()
{
  MutexGuard mg(mutex_);
  isStopping_ = true;
}

bool PersServer::isStopped() {
  MutexGuard mg(mutex_);
  return isStopping_;
}

void PersServer::init(const char *host, int port)
{
  logger = Logger::getInstance("acceptor");
  smsc_log_info(logger, "init pers server");

  if (masterSocket_.InitServer(host, port, 0, 0) == -1) {          
    smsc_log_error(logger, "failed to init master socket");
    throw Exception("Socket::InitServer() failed");
  }
  if (masterSocket_.StartServer() == -1) {
    smsc_log_error(logger, "failed to start master socket");
    throw Exception("Socket::StartServer() failed");
  }
  smsc_log_debug(logger, "Pers Server inited on %s:%d", host, port);
  {
    MutexGuard mg(mutex_);
    isStopping_ = false;
  }
}

}//pvss
}//scag2

