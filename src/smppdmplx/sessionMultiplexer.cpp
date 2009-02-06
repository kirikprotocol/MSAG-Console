#include <smppdmplx/core_ax/network/SocketPool.hpp>
#include <smppdmplx/core_ax/network/Socket.hpp>
#include <util/Exception.hpp>

#include "RawMessage.hpp"
#include "SMPP_MessageFactory.hpp"

#include "CacheOfIncompleteReadMessages.hpp"

#include "Publisher.hpp"

#include "SocketPool_Singleton.hpp"
#include "Configuration.hpp"
#include "PendingOutDataQueue.hpp"
#include "SessionHelper.hpp"
#include "NetworkException.hpp"
#include "IdleSocketsPool.hpp"
#include "SMPP_EnquireLink.hpp"
#include "BufferedOutputStream.hpp"
#include "PendingOutDataQueue.hpp"

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;


static void
acceptConnection(smsc::core_ax::network::SocketPool& socketPool,
                 smsc::core_ax::network::ServerSocket& listenSocket)
{
  smsc::core_ax::network::Socket connectedSocket = listenSocket.accept();
  smsc_log_debug(dmplxlog,"multiplexer::: connect accepted");
  connectedSocket.setNonBlocking();
  // Помещаем присоединенный сокет в пул
  socketPool.push_socket(connectedSocket);
  // Вернули в пул сокет, на котором прослушиваем запросы
  // на новые соединения
  socketPool.push_socket(listenSocket);
  smpp_dmplx::IdleSocketsPool::getInstance().insertWaitingSocket(connectedSocket, smpp_dmplx::Configuration::getInstance().inactivityPeriod);
}

static void
getDataFromSocket(smsc::core_ax::network::SocketPool& socketPool,
                  smsc::core_ax::network::Socket& readySocket)
{
  smpp_dmplx::CacheOfIncompleteReadMessages& cacheOfIncompleteReadMessages = smpp_dmplx::CacheOfIncompleteReadMessages::getInstance();
  try {
    smpp_dmplx::IdleSocketsPool::getInstance().removeActiveSocket(readySocket);

    socketPool.push_socket(readySocket);

    smpp_dmplx::RawMessage& rawMessage = 
      cacheOfIncompleteReadMessages.getIncompleteMessageForSocket(readySocket);

    if ( !rawMessage.haveReadLength() ) {
      rawMessage.readDataLength(readySocket.getSocketDescriptor());
      smpp_dmplx::IdleSocketsPool::getInstance().insertWaitingSocket(readySocket, smpp_dmplx::Configuration::getInstance().inactivityPeriod);
      return;
    }
    if ( !rawMessage.haveReadRawData() ) {
      smsc_log_debug(dmplxlog,"getDataFromSocket::: call to rawMessage.readRawData(readySocket)");
      rawMessage.readRawData(readySocket.getSocketDescriptor());
      if ( !rawMessage.haveReadRawData() ) {
        smpp_dmplx::IdleSocketsPool::getInstance().insertWaitingSocket(readySocket, smpp_dmplx::Configuration::getInstance().inactivityPeriod);
        return;
      }
    }
    smsc_log_info(dmplxlog,"getDataFromSocket::: packet data has been read");
    smpp_dmplx::BufferedInputStream  messageBuffer = rawMessage.getRawData();
    smsc_log_debug(dmplxlog,"getDataFromSocket::: Remove message from cache of incomplete messages");
    cacheOfIncompleteReadMessages.removeCompleteMessageForSocket(readySocket);

    smsc_log_debug(dmplxlog,"getDataFromSocket::: Try create SMPP message from raw data");
    std::auto_ptr<smpp_dmplx::SMPP_message> smpp(smpp_dmplx::SMPP_MessageFactory::getInstance().createSmppMessage(messageBuffer));
    smsc_log_debug(dmplxlog,"getDataFromSocket::: SMPP message has been created");
    smpp_dmplx::IdleSocketsPool::getInstance().insertWaitingSocket(readySocket, smpp_dmplx::Configuration::getInstance().inactivityPeriod);
    // Publish message to registered subscribers.
    // Message processing is performed in one of registred subscribers.
    smpp_dmplx::Publisher::getInstance().publish(smpp, readySocket);
  } catch (smpp_dmplx::EOFException& ex) {
    smsc_log_error(dmplxlog,"getDataFromSocket::: Catched EOFException [%s]", ex.what());
    smpp_dmplx::SessionHelper::dropActiveSession(readySocket);
    cacheOfIncompleteReadMessages.removeCompleteMessageForSocket(readySocket);
    throw;
  } catch (smpp_dmplx::NetworkException& ex) {
    smsc_log_error(dmplxlog,"getDataFromSocket::: Catched NetworkException [%s]", ex.what());
    smpp_dmplx::SessionHelper::dropActiveSession(readySocket);
    cacheOfIncompleteReadMessages.removeCompleteMessageForSocket(readySocket);
    throw;
  }
}

static void
processInactiveConnections()
{
  smpp_dmplx::IdleSocketsPool::IdleSocketList_t timedOutSocketlist;
  if ( smpp_dmplx::IdleSocketsPool::getInstance().getTimedOutSocketsList(&timedOutSocketlist) == true ) {
    for (std::list<smpp_dmplx::IdleSocketsPool::Socket_Timeout_pair_t>::iterator iter = timedOutSocketlist.begin(); iter != timedOutSocketlist.end(); ++iter) {
      smsc_log_info(dmplxlog,"processInactiveConnections::: timeout has been occured on connection [%s]. Close connection.", iter->first.toString().c_str());

      smsc::core_ax::network::Socket socketToRemove = iter->first;
      smpp_dmplx::SessionHelper::dropActiveSession(socketToRemove);
      smpp_dmplx::CacheOfIncompleteReadMessages::getInstance().removeCompleteMessageForSocket(socketToRemove);
    }
  }
}

static void
sendDataToPeer(smsc::core_ax::network::Socket& readySocket)
{
  try {
    smpp_dmplx::PendingOutDataQueue::getInstance().sendScheduledData(readySocket);
  } catch (smpp_dmplx::NetworkException& ex) {
    smsc_log_error(dmplxlog,"multiplexer::: Catched NetworkException exception [%s]", ex.what());
    smpp_dmplx::SessionHelper::dropActiveSession(readySocket);
    smpp_dmplx::CacheOfIncompleteReadMessages::getInstance().removeCompleteMessageForSocket(readySocket);
    throw;
  }
}

static void
bypassReadableSockets(smsc::core_ax::network::ServerSocket& listenSocket,
                      smsc::core_ax::network::SocketPool& socketPool,
                      smsc::core_ax::network::SocketPool::SocketList_t& readySocketForRead_list)
{
  for (smsc::core_ax::network::SocketPool::SocketList_t::iterator
         readySocket_iter = readySocketForRead_list.begin(); readySocket_iter != readySocketForRead_list.end(); ++readySocket_iter) {
    if ( *readySocket_iter == listenSocket )
      acceptConnection(socketPool, listenSocket);
    else
      getDataFromSocket(socketPool, *readySocket_iter);
  }
}

static void
bypassWritableSockets(smsc::core_ax::network::SocketPool::SocketList_t& readySocketForWrite_list)
{
  // Обходим список сокетов готовых на запись
  for (smsc::core_ax::network::SocketPool::SocketList_t::iterator
         readySocket_iter = readySocketForWrite_list.begin(); readySocket_iter != readySocketForWrite_list.end(); ++readySocket_iter)
    sendDataToPeer(*readySocket_iter);
}

void multiplexer()
{
  try {
    smsc_log_debug(dmplxlog,"multiplexer::: create ServerSocket(port=%d)", smpp_dmplx::Configuration::getInstance().listenPort);

    smsc::core_ax::network::ServerSocket listenSocket(smpp_dmplx::Configuration::getInstance().listenPort);

    smsc::core_ax::network::SocketPool& socketPool = smpp_dmplx::SocketPool_Singleton::getInstance();
    socketPool.setTimeOut(smpp_dmplx::Configuration::getInstance().selectTimeOut);

    smsc_log_debug(dmplxlog,"multiplexer::: push ServerSocket to socketPool");
    socketPool.push_socket(listenSocket);

    while(true) {
      smsc_log_debug(dmplxlog,"multiplexer::: call to socketPool->listen");

      try {
        smsc::core_ax::network::SocketPool::SocketList_t readySocketForRead_list;
        smsc::core_ax::network::SocketPool::SocketList_t readySocketForWrite_list;

        smsc::core_ax::network::SocketPool::listen_status_t 
          listenStatus = socketPool.listen(readySocketForRead_list, readySocketForWrite_list);

        smsc_log_debug(dmplxlog,"multiplexer::: socketPool->listen has returned listenStatus=%d", listenStatus);
        if ( listenStatus != smsc::core_ax::network::SocketPool::TIMEOUT ) {
          bypassReadableSockets(listenSocket, socketPool, readySocketForRead_list);

          bypassWritableSockets(readySocketForWrite_list);
        }

        processInactiveConnections();
      } catch (smpp_dmplx::NetworkException& ex) {}
    }

  } catch (std::exception& ex) {
    smsc_log_error(dmplxlog,"multiplexer::: Catch unexpected exception [%s]. Terminated.", ex.what());
  }
}
