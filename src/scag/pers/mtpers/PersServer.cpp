#include "PersServer.h"

namespace scag { namespace mtpers {

using smsc::util::Exception;

PersServer::PersServer(IOTaskManager& _iomanager):iomanager_(_iomanager)
{
}

int PersServer::Execute()
{
  Socket *clientSocket;

  smsc_log_debug(logger, "pers server started");

  for (;;) {

    clientSocket = masterSocket_.Accept();

    if (isStopping_)
        break;

    if (!clientSocket) {
        smsc_log_error(logger, "failed to accept, error: %s", strerror(errno));
        break;
    }
    ConnectionContext *cx = new ConnectionContext(clientSocket);
    smsc_log_debug(logger, "Client connected socket:%p context:%p", clientSocket, cx);
    if (!iomanager_.process(cx)) {
      clientSocket->Write("SB", 2);
      smsc_log_warn(logger, "Server Busy sent. Disconnected");
      delete cx;
    }
    //TODO: check license?
  }
  if (clientSocket) {
    delete clientSocket;
  }

  smsc_log_debug(logger, "quit");

  return isStopping_ == false;
}

const char* PersServer::taskName()
{
  return "MTPersServer";
}

void PersServer::shutdown()
{
  isStopping_ = true;

  masterSocket_.Close();
  //WaitFor();
  iomanager_.shutdown();
}

void PersServer::init(const char *host, int port)
{
  isStopping_ = false;

  logger = Logger::getInstance("persserver");

  if (masterSocket_.InitServer(host, port, 0, 0) == -1) {          
    smsc_log_error(logger, "failed to init master socket");
    throw Exception("Socket::InitServer() failed");
  }
  if (masterSocket_.StartServer() == -1) {
    smsc_log_error(logger, "failed to start master socket");
    throw Exception("Socket::StartServer() failed");
  }
  smsc_log_debug(logger, "Pers Server inited on %s:%d", host, port);

  //Start();
}

}//mtpers
}//scag

