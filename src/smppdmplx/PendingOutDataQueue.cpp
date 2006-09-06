#include <unistd.h>
#include "PendingOutDataQueue.hpp"
#include "SocketPool_Singleton.hpp"
#include <util/Exception.hpp>

#include <logger/Logger.h>
extern smsc::logger::Logger* dmplxlog;

smpp_dmplx::PendingOutDataQueue::sched_table_t
smpp_dmplx::PendingOutDataQueue::sched_table;

void
smpp_dmplx::PendingOutDataQueue::scheduleDataForSending(BufferedOutputStream& outBuf, smsc::core_ax::network::Socket& socket)
{
  smsc_log_info(dmplxlog,"PendingOutDataQueue::scheduleDataForSending::: put data to output queue for socket=[%s], data size=%d", socket.toString().c_str(), outBuf.getSize());

  sched_table_t::iterator iter = sched_table.find(socket);
  if ( iter != sched_table.end() ) {
    smsc_log_debug(dmplxlog,"PendingOutDataQueue::scheduleDataForSending::: add outstanding data in output queue");
    smsc_log_debug(dmplxlog,"PendingOutDataQueue::scheduleDataForSending::: size of added data = %d",outBuf.getSize());
    iter->second.push_back(data_t(outBuf, outBuf.getSize()));
    smsc_log_debug(dmplxlog,"PendingOutDataQueue::scheduleDataForSending::: push socket=[%s] in writeable mask", socket.toString().c_str());
    SocketPool_Singleton::getInstance().push_socket(socket, smsc::core_ax::network::SocketPool::WAIT_WRITEABLE);
  } else {
    smsc_log_debug(dmplxlog,"PendingOutDataQueue::scheduleDataForSending::: insert outstanding data in output queue");
    std::pair<sched_table_t::iterator, bool> ins_result = sched_table.insert(std::make_pair(socket, scheduled_data_t()));
    if ( ins_result.second ) {
      smsc_log_debug(dmplxlog,"PendingOutDataQueue::scheduleDataForSending::: size of inserted data = %d",outBuf.getSize());
      ins_result.first->second.push_back(data_t(outBuf, outBuf.getSize()));
      smsc_log_debug(dmplxlog,"PendingOutDataQueue::scheduleDataForSending::: push socket=[%s] in writeable mask", socket.toString().c_str());
      SocketPool_Singleton::getInstance().push_socket(socket, smsc::core_ax::network::SocketPool::WAIT_WRITEABLE);
    }
  }
}

void
smpp_dmplx::PendingOutDataQueue::sendScheduledData(smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(dmplxlog,"PendingOutDataQueue::sendScheduledData::: get data from output queue for socket=[%s]", socket.toString().c_str());
  sched_table_t::iterator iter = sched_table.find(socket);
  if ( iter != sched_table.end() ) {
    if ( ! iter->second.empty() ) {
      smsc_log_debug(dmplxlog,"PendingOutDataQueue::sendScheduledData::: data queue is not empty");
      data_t dataForSend= iter->second.front();
      smsc_log_info(dmplxlog,"PendingOutDataQueue::sendScheduledData::: sending data size=%d, fullBufferSize=%d, bufPos=%d", dataForSend.fullBufferSize - dataForSend.pos, dataForSend.fullBufferSize, dataForSend.pos);

      ssize_t sz = ::write(socket.getSocketDescriptor(), dataForSend.buffer + dataForSend.pos, dataForSend.fullBufferSize - dataForSend.pos);
      if ( sz < 0 )
        throw smsc::util::Exception("PendingOutDataQueue::sendScheduledData::: write failed");
      smsc_log_info(dmplxlog,"PendingOutDataQueue::sendScheduledData::: wrote %d bytes", sz);
      dataForSend.pos += sz;
      if ( dataForSend.pos == dataForSend.fullBufferSize ) {
        smsc_log_info(dmplxlog,"PendingOutDataQueue::sendScheduledData::: full buffer was wrote. remove buffer from queue");
        delete [] dataForSend.buffer;
        iter->second.pop_front();
        if ( !iter->second.empty() ) {
          smsc_log_debug(dmplxlog,"PendingOutDataQueue::sendScheduledData::: data queue is not empty - put socket=[%s] to write mask", socket.toString().c_str());
          SocketPool_Singleton::getInstance().push_socket(socket, smsc::core_ax::network::SocketPool::WAIT_WRITEABLE);
        } else {
          smsc_log_debug(dmplxlog,"PendingOutDataQueue::sendScheduledData::: data queue is empty - remove socket=[%s] from sched_table", iter->first.toString().c_str());
          sched_table.erase(iter);
        }
      }
    } else {
      smsc_log_debug(dmplxlog,"PendingOutDataQueue::sendScheduledData::: data queue is empty - remove socket=[%s] from sched_table", iter->first.toString().c_str());
      sched_table.erase(iter);
    }
  }
}
