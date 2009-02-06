#include <unistd.h>
#include "PendingOutDataQueue.hpp"
#include "SocketPool_Singleton.hpp"
#include "NetworkException.hpp"

namespace smpp_dmplx {

PendingOutDataQueue::PendingOutDataQueue()
  : _log(smsc::logger::Logger::getInstance("pendq")) {}

void
PendingOutDataQueue::scheduleDataForSending(BufferedOutputStream& outBuf, smsc::core_ax::network::Socket& socket)
{
  smsc_log_info(_log,"PendingOutDataQueue::scheduleDataForSending::: put data to output queue for socket=[%s], data size=%d", socket.toString().c_str(), outBuf.getSize());

  sched_table_t::iterator iter = _sched_table.find(socket);

  if ( iter == _sched_table.end() ) {
    std::pair<sched_table_t::iterator, bool> ins_result = _sched_table.insert(std::make_pair(socket, scheduled_data_t()));
    if ( !ins_result.second )
      throw smsc::util::Exception("PendingOutDataQueue::scheduleDataForSending::: can't insert element into sche_table");
    iter = ins_result.first;
  }

  smsc_log_debug(_log,"PendingOutDataQueue::scheduleDataForSending::: add outstanding data in output queue, size of data = %d",outBuf.getSize());
  iter->second.push_back(data_t(outBuf, outBuf.getSize()));
  smsc_log_debug(_log,"PendingOutDataQueue::scheduleDataForSending::: push socket=[%s] to writeable mask", socket.toString().c_str());
  SocketPool_Singleton::getInstance().push_socket(socket, smsc::core_ax::network::SocketPool::WAIT_WRITEABLE);
}

void
PendingOutDataQueue::sendScheduledData(smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(_log,"PendingOutDataQueue::sendScheduledData::: get data from output queue for socket=[%s]", socket.toString().c_str());
  sched_table_t::iterator iter = _sched_table.find(socket);
  if ( iter != _sched_table.end() ) {
    if ( ! iter->second.empty() ) {
      smsc_log_debug(_log,"PendingOutDataQueue::sendScheduledData::: data queue is not empty");
      data_t dataForSend= iter->second.front();
      smsc_log_info(_log,"PendingOutDataQueue::sendScheduledData::: sending data size=%d, fullBufferSize=%d, bufPos=%d", dataForSend.fullBufferSize - dataForSend.pos, dataForSend.fullBufferSize, dataForSend.pos);

      ssize_t sz = ::write(socket.getSocketDescriptor(), dataForSend.buffer + dataForSend.pos, dataForSend.fullBufferSize - dataForSend.pos);
      if ( sz < 0 )
        throw NetworkException("PendingOutDataQueue::sendScheduledData::: write failed");
      smsc_log_info(_log,"PendingOutDataQueue::sendScheduledData::: wrote %d bytes", sz);
      dataForSend.pos += sz;
      if ( dataForSend.pos == dataForSend.fullBufferSize ) {
        smsc_log_info(_log,"PendingOutDataQueue::sendScheduledData::: full buffer has been written. remove buffer from queue");
        delete [] dataForSend.buffer;
        iter->second.pop_front();
        if ( !iter->second.empty() ) {
          smsc_log_debug(_log,"PendingOutDataQueue::sendScheduledData::: data queue is not empty - put socket=[%s] to write mask", socket.toString().c_str());
          SocketPool_Singleton::getInstance().push_socket(socket, smsc::core_ax::network::SocketPool::WAIT_WRITEABLE);
        } else {
          smsc_log_debug(_log,"PendingOutDataQueue::sendScheduledData::: data queue is empty - remove socket=[%s] from sched_table", iter->first.toString().c_str());
          _sched_table.erase(iter);
        }
      }
    } else {
      smsc_log_debug(_log,"PendingOutDataQueue::sendScheduledData::: data queue is empty - remove socket=[%s] from sched_table", iter->first.toString().c_str());
      _sched_table.erase(iter);
    }
  }
}

void
PendingOutDataQueue::cancelScheduledData(const smsc::core_ax::network::Socket& socket)
{
  smsc_log_debug(_log,"PendingOutDataQueue::cancelScheduledData::: cancel scheduled data for socket=[%s]", socket.toString().c_str());
  sched_table_t::iterator iter = _sched_table.find(socket);
  if ( iter != _sched_table.end() ) {
    scheduled_data_t& schedData = iter->second;
    while ( !schedData.empty() ) {
      data_t dataForSend= schedData.front();
      delete [] dataForSend.buffer;
      schedData.pop_front();
    }
    _sched_table.erase(iter);
    SocketPool_Singleton::getInstance().remove_socket(socket, smsc::core_ax::network::SocketPool::WAIT_WRITEABLE);
    smsc_log_debug(_log,"PendingOutDataQueue::cancelScheduledData::: scheduled data has been canceled for socket=[%s]", socket.toString().c_str());
  }
}

}
