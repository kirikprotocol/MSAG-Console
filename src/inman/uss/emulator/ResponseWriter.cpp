#include <memory>
#include <utility>
#include "ResponseWriter.hpp"

namespace smsc {
namespace inman {
namespace uss {

int
ResponseWriter::Execute()
{
  while(true) {
    output_item oItem = getReadyToDeliveryResponse();

    std::auto_ptr<interaction::SPckUSSResult> ptrCleaner(oItem.result);
    
    connect_info* connInfo;
    {
      core::synchronization::MutexGuard synchronize (_connRegistryLock);
      connect_registry_t::iterator iter = _connectRegistry.find(oItem.connId);
      if ( iter == _connectRegistry.end() )
        continue;
      connInfo = iter->second;
      connInfo->connectLock.Lock();
    }
    smsc_log_debug(_logger, "ResponseWriter::Execute::: process output_item=[%s]", oItem.toString().c_str());
    int st=0;
    try {
      st = connInfo->connect->sendPck(oItem.result);
    } catch (...) { connInfo->connectLock.Unlock(); throw; }

    connInfo->connectLock.Unlock();
    if ( st == -1 )
      cancelScheduledResponse(oItem.connId);
  }
  return 0;
}

void
ResponseWriter::scheduleResponse(smsc::inman::interaction::SPckUSSResult* result,
                                 inman::interaction::Connect* conn)
{
  unsigned connId = conn->getId();
  {
    core::synchronization::MutexGuard synchronize (_connRegistryLock);
    if ( _connectRegistry.find(connId) == _connectRegistry.end() )
      _connectRegistry.insert(std::make_pair(connId, new connect_info(conn)));
  }
  
  struct timeval timeToAcivate;
  gettimeofday(&timeToAcivate, NULL);
  long sec = _responseDelayInMsecs / 1000L;
  long usec = (_responseDelayInMsecs % 1000L) * 1000L;
  timeToAcivate.tv_sec += sec;
  timeToAcivate.tv_usec += usec;

  core::synchronization::MutexGuard synchronize(_eventMonitor);
  _output_queue.push_back(output_item(timeToAcivate, result, connId));
  _eventMonitor.notify();
}

void
ResponseWriter::cancelScheduledResponse(unsigned connId)
{
  smsc_log_debug(_logger, "ResponseWriter::cancelScheduledResponse::: cancel connection with id=%d", connId);
  core::synchronization::MutexGuard synchronize(_connRegistryLock);
  connect_registry_t::iterator iter = _connectRegistry.find(connId);
  if ( iter == _connectRegistry.end() )
    return;

  smsc_log_debug(_logger, "ResponseWriter::cancelScheduledResponse::: connection with id=%d was found, cancel it", connId);
  connect_info* connInfo = iter->second;
  connInfo->connectLock.Lock();
  // if we acquired this lock then guaranted nobody else send message over this connection at the same time
  connInfo->connectLock.Unlock();
  _connectRegistry.erase(iter);
}

ResponseWriter::output_item
ResponseWriter::getReadyToDeliveryResponse() {
  core::synchronization::MutexGuard synchronize(_eventMonitor);
  while(_output_queue.empty())
    _eventMonitor.wait();
  output_item nextOutputItem =_output_queue.front();
  _output_queue.pop_front();
  struct timeval currentTime;
  gettimeofday(&currentTime, NULL);
  if ( currentTime.tv_sec > nextOutputItem.timeToAcivate.tv_sec ||
       (currentTime.tv_sec == nextOutputItem.timeToAcivate.tv_sec &&
        currentTime.tv_usec >= nextOutputItem.timeToAcivate.tv_usec) )
    return nextOutputItem;
  else {
    struct timeval timeout;
    timeout.tv_sec = nextOutputItem.timeToAcivate.tv_sec - currentTime.tv_sec;
    timeout.tv_usec = nextOutputItem.timeToAcivate.tv_usec - currentTime.tv_usec;
    select(0, NULL, NULL, NULL, &timeout);
    return nextOutputItem;
  }
}

}}}
