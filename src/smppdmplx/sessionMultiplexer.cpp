#include <core_ax/network/SocketPool.hpp>
#include <core_ax/network/Socket.hpp>
#include <util/Exception.hpp>
//#include <core_ax/network/NetworkException.hpp>

//#include "SMPP_message.hpp"
#include "RawMessage.hpp"
#include "SMPP_MessageFactory.hpp"

#include "CacheOfIncompleteReadMessages.hpp"

#include "Publisher.hpp"
//#include "BufferedOutputStream.hpp"
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

uint32_t globalSeqNum=0;

void multiplexer()
{
  try {
    const int listeningPort = smpp_dmplx::Configuration::getInstance().listenPort;
    const time_t enqLinkTimeoutValue=smpp_dmplx::Configuration::getInstance().enqLinkPeriod;
    const time_t reqWaitTimeout=smpp_dmplx::Configuration::getInstance().selectTimeOut;

    smsc_log_debug(dmplxlog,"multiplexer::: create ServerSocket(port=%d)", listeningPort);

    smsc::core_ax::network::ServerSocket listenSocket(listeningPort);

    smsc::core_ax::network::SocketPool& socketPool = SocketPool_Singleton::getInstance();
    socketPool.setTimeOut(reqWaitTimeout);

    smsc_log_debug(dmplxlog,"multiplexer::: socketPool = 0x%p", &socketPool);
    smsc_log_debug(dmplxlog,"multiplexer::: push ServerSocket to socketPool");
    socketPool.push_socket(listenSocket);

    smpp_dmplx::CacheOfIncompleteReadMessages cacheOfIncompleteReadMessages;
    while(true) {
      smsc_log_debug(dmplxlog,"multiplexer::: call to socketPool->listen");

      smsc::core_ax::network::SocketPool::SocketList_t readySocketForRead_list;
      smsc::core_ax::network::SocketPool::SocketList_t readySocketForWrite_list;

      smsc::core_ax::network::SocketPool::listen_status_t 
        listenStatus = socketPool.listen(readySocketForRead_list, readySocketForWrite_list);

      smsc_log_debug(dmplxlog,"multiplexer::: socketPool->listen returned");
      if ( listenStatus != smsc::core_ax::network::SocketPool::TIMEOUT ) {
        // Обходим список сокетов готовых на чтение
        for (smsc::core_ax::network::SocketPool::SocketList_t::iterator
               readySocket_iter = readySocketForRead_list.begin(); readySocket_iter != readySocketForRead_list.end(); ++readySocket_iter) {
          if ( *readySocket_iter == listenSocket ) {
            smsc_log_debug(dmplxlog,"multiplexer::: readySocket == listenSocket - cast Socket to ServerSocket");
            smsc::core_ax::network::ServerSocket serverSocket(*readySocket_iter);
            smsc_log_debug(dmplxlog,"multiplexer::: call to serverSocket->accept");
            smsc::core_ax::network::Socket connectedSocket
              = serverSocket.accept();
            smsc_log_debug(dmplxlog,"multiplexer::: connect accepted");
            connectedSocket.setNonBlocking();
            // Помещаем присоединенный сокет в пул
            socketPool.push_socket(connectedSocket);
            // Вернули в пул сокет, на котором прослушиваем запросы
            // на новые соединения
            socketPool.push_socket(listenSocket);
            smpp_dmplx::IdleSocketsPool::getInstance().insertWaitingSocket(connectedSocket, enqLinkTimeoutValue);
          } else {
            smsc_log_debug(dmplxlog,"multiplexer::: has readySocket");
            try {
              smsc_log_debug(dmplxlog,"multiplexer::: readySocket is readable");
              smpp_dmplx::IdleSocketsPool::getInstance().removeActiveSocket(*readySocket_iter);
              socketPool.push_socket(*readySocket_iter);
              smsc_log_debug(dmplxlog,"multiplexer::: call to cacheOfIncompleteReadMessages.getIncompleteMessageForSocket(readySocket)");
              smpp_dmplx::RawMessage& rawMessage = 
                cacheOfIncompleteReadMessages.getIncompleteMessageForSocket(*readySocket_iter);

              smsc_log_debug(dmplxlog,"multiplexer::: check rawMessage.haveReadLenght");
              if ( !rawMessage.haveReadLenght() ) {
                smsc_log_debug(dmplxlog,"multiplexer::: call to rawMessage.readDataLength(readySocket)");
                rawMessage.readDataLength((*readySocket_iter).getSocketDescriptor());
                smpp_dmplx::IdleSocketsPool::getInstance().insertWaitingSocket(*readySocket_iter, enqLinkTimeoutValue);
                continue;
              }
              smsc_log_debug(dmplxlog,"multiplexer::: check rawMessage.haveReadRawData()");
              if ( !rawMessage.haveReadRawData() ) {
                smsc_log_debug(dmplxlog,"multiplexer::: call to rawMessage.readRawData(readySocket)");
                rawMessage.readRawData((*readySocket_iter).getSocketDescriptor());
                if ( !rawMessage.haveReadRawData() ) {
                  smpp_dmplx::IdleSocketsPool::getInstance().insertWaitingSocket(*readySocket_iter, enqLinkTimeoutValue);
                  continue;
                }
              }
              smsc_log_debug(dmplxlog,"multiplexer::: call to rawMessage.getRawData");
              smpp_dmplx::BufferedInputStream
                messageBuffer = rawMessage.getRawData();
              smsc_log_debug(dmplxlog,"multiplexer::: Try remove message from cache of incomple messages");
              cacheOfIncompleteReadMessages.removeCompleteMessageForSocket(*readySocket_iter);
              smsc_log_debug(dmplxlog,"multiplexer::: It was removed message from cache of incomple messages");
              //Congratulations! The message was read from network.
              smsc_log_debug(dmplxlog,"multiplexer::: Try create SMPP message from raw data");
              std::auto_ptr<smpp_dmplx::SMPP_message> smpp(smpp_dmplx::SMPP_MessageFactory::getInstance().createSmppMessage(messageBuffer));
              smsc_log_debug(dmplxlog,"multiplexer::: SMPP message was created");
              smpp_dmplx::IdleSocketsPool::getInstance().insertWaitingSocket(*readySocket_iter, enqLinkTimeoutValue);
              // To publish message to registred subscribers.
              // Message processing is performed in one of registred subscribers.
              smpp_dmplx::Publisher::getInstance().publish(smpp, *readySocket_iter);
            } catch (smpp_dmplx::EOFException& ex) {
              smsc_log_error(dmplxlog,"multiplexer::: Catch exception [%s]", ex.what());
              smpp_dmplx::SessionHelper::dropActiveSession(*readySocket_iter);
              cacheOfIncompleteReadMessages.removeCompleteMessageForSocket(*readySocket_iter);
            } catch (std::exception& ex) {
              smsc_log_error(dmplxlog,"multiplexer::: Catch exception [%s]", ex.what());
              smpp_dmplx::SessionHelper::dropActiveSession(*readySocket_iter);
              cacheOfIncompleteReadMessages.removeCompleteMessageForSocket(*readySocket_iter);
            }
          }
        }

        // Обходим список сокетов готовых на запись
        for (smsc::core_ax::network::SocketPool::SocketList_t::iterator
               readySocket_iter = readySocketForWrite_list.begin(); readySocket_iter != readySocketForWrite_list.end(); ++readySocket_iter) {
          // Для каждого сокета готового на запись вызываем метод sendScheduledData
          // класса  PendingOutDataQueue для записи подготовленных для 
          // сокета данных.
          try {
            smpp_dmplx::PendingOutDataQueue::sendScheduledData(*readySocket_iter);
          } catch (std::exception& ex) {
            smsc_log_error(dmplxlog,"multiplexer::: Catch unexpected exception [%s]", ex.what());
            smpp_dmplx::SessionHelper::dropActiveSession(*readySocket_iter);
            cacheOfIncompleteReadMessages.removeCompleteMessageForSocket(*readySocket_iter);
          }
        }
      }
      
      smpp_dmplx::IdleSocketsPool::IdleSocketList_t timedOutSocketlist;
      if ( smpp_dmplx::IdleSocketsPool::getInstance().getTimedOutSocketsList(timedOutSocketlist) == true ) {
        for (std::list<smpp_dmplx::IdleSocketsPool::Socket_Timeout_pair_t>::iterator 
               iter = timedOutSocketlist.begin(); iter != timedOutSocketlist.end(); ++iter) {
          // послать EnquireLink
          smpp_dmplx::SMPP_EnquireLink enqLinkReq;
          enqLinkReq.setSequenceNumber(globalSeqNum++);
          smsc_log_info(dmplxlog,"multiplexer::: Write EnquireLink request=[%s] to socket=[%s]", enqLinkReq.toString().c_str(), iter->first.toString().c_str());
          std::auto_ptr<smpp_dmplx::BufferedOutputStream> outBuf = enqLinkReq.marshal();
          smpp_dmplx::PendingOutDataQueue::scheduleDataForSending(*outBuf,iter->first);
        }
      }
    }

  } catch (std::exception& ex) {
    smsc_log_error(dmplxlog,"multiplexer::: Catch unexpected exception [%s]. Terminated.", ex.what());
  }
}
