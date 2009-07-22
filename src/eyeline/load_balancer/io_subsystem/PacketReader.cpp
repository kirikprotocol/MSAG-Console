#include <sys/types.h>
#include <time.h>
#include <memory>

#include "IOProcessor.hpp"
#include "PacketReader.hpp"
#include "GotPacketEvent.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

PacketReader::PacketReader(IOProcessor& io_processor,
                           MessagePublisher& message_publisher,
                           unsigned max_streams_per_reader)
  : _logger(smsc::logger::Logger::getInstance("io_subsystem")),
    _ioProcessor(io_processor), _shutdownInProgress(false),
    _messagePublisher(message_publisher), _newSchedTasks(max_streams_per_reader)
{
  _notificationEndpoint = _pipe.getLeftSide()->getOutputStream();
  _signallingEndpoint = _pipe.getRightSide()->getInputStream();
  _readableIOObjectsPool.insert(_signallingEndpoint);
}

int
PacketReader::Execute()
{
  smsc_log_debug(_logger, "PacketReader::Execute::: thread stared");
  while (!_shutdownInProgress) {
    try {
      int listenStatus = _readableIOObjectsPool.listen();
      if ( listenStatus & IOObjectsPool_notMTSafe::OK_READ_READY ) {
        corex::io::InputStream* iStream = _readableIOObjectsPool.getNextReadyInputStream();
        while ( iStream ) {
          if ( iStream == _signallingEndpoint )
            processInputStreamManipulationTask();
          else
            receivePacketAndPublishIt(iStream);
          iStream = _readableIOObjectsPool.getNextReadyInputStream();
        }
      } else {
        smsc_log_error(_logger, "PacketReader::Execute::: unexpected return (%d) from call to IOObjectsPool::listen",
                       listenStatus);
      }
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "PacketReader::Execute::: caught unexpected exception [%s]",
                     ex.what());
    }
  }
  smsc_log_info(_logger, "PacketReader::Execute::: thread exited");
  return 0;
}

void
PacketReader::shutdown()
{
  smsc_log_info(_logger, "try shutdown PacketReader");
  _shutdownInProgress = true;
  scheduleTask(TaskType(SHUTDOWN));
}

void
PacketReader::addInputStream(const LinkId& link_id)
{
  {
    smsc::core::synchronization::MutexGuard synchronize(_lockForKnownIStreams);
    istream_registry_t::iterator iter = _known_iStreams.find(link_id);
    if ( iter != _known_iStreams.end() ) {
      smsc_log_error(_logger, "PacketReader::addInputStream::: input stream for link with id='%s' already exists",
                     link_id.toString().c_str());
      return;
    }
  }

  smsc_log_debug(_logger, "PacketReader::addInputStream::: try add input stream for link with id='%s'",
                 link_id.toString().c_str());
  scheduleTask(TaskType(ADD_ISTREAM, link_id));
}

void
PacketReader::removeInputStream(const LinkId& link_id)
{
  {
    smsc::core::synchronization::MutexGuard synchronize(_lockForKnownIStreams);
    istream_registry_t::iterator iter = _known_iStreams.find(link_id);
    if ( iter == _known_iStreams.end() )
      return;
  }

  smsc_log_info(_logger, "PacketReader::removeInputStream::: try remove input stream for link with id='%s'",
                link_id.toString().c_str());
  scheduleTask(TaskType(REMOVE_ISTREAM, link_id));
}

void
PacketReader::scheduleTask(const TaskType& task)
{
  _newSchedTasks.enqueue(task);

  uint8_t event=task.taskType;
  try {
    _notificationEndpoint->write(&event, sizeof(event));
  } catch (...) {}
}

void
PacketReader::receivePacketAndPublishIt(corex::io::InputStream* i_stream)
{
  LinkId linkId(i_stream->getOwner()->getId());

  LinkRefPtr link = _ioProcessor.getLink(linkId);

  if ( link.Get() ) {
    std::auto_ptr<Packet> packet(NULL);
    try {
      packet.reset(link->receive());

      if ( !packet.get() ) return;
    } catch (corex::io::EOFException& ex) {
      smsc_log_info(_logger, "PacketReader::receivePacketAndPublishIt::: caught EOFException for link with id=%s",
                    linkId.toString().c_str());
      cleanUpBrokenConnection(link.Get());
      return;
    }

    GotPacketEvent* newGotPacketEvent = new GotPacketEvent(packet.release(),
                                                           _ioProcessor, linkId);
    _messagePublisher.publish(newGotPacketEvent);
  } else {
    smsc_log_error(_logger, "PacketReader::receivePacketAndPublishIt::: link not found for id='%s', remove corresponding input stream from pool",
                   linkId.toString().c_str());
    _removeInputStream(linkId);
  }
}

void
PacketReader::cleanUpBrokenConnection(Link* link)
{
  const LinkId& linkId = link->getLinkId();
  _removeInputStream(linkId);
  _ioProcessor.cleanUpConnection(linkId);
}

void
PacketReader::processInputStreamManipulationTask()
{
  uint8_t event=0;
  _signallingEndpoint->read(&event, sizeof(event));

  assert(event == ADD_ISTREAM ||
         event == REMOVE_ISTREAM ||
         event == SHUTDOWN);

  const TaskType& task = _newSchedTasks.dequeue();
  assert(event == task.taskType);
  if ( task.taskType == ADD_ISTREAM )
    _addInputStream(task.linkId);
  else if ( task.taskType == REMOVE_ISTREAM )
    _removeInputStream(task.linkId);
  else
    return;
}

void
PacketReader::_addInputStream(const LinkId& added_link_id)
{
  LinkRefPtr link = _ioProcessor.getLink(added_link_id);

  if ( !link.Get() )
    return;

  corex::io::InputStream* iStream;
  {
    smsc::core::synchronization::MutexGuard synchronize(_lockForKnownIStreams);
    if ( _known_iStreams.find(added_link_id) != _known_iStreams.end() )
      return;
    iStream = link->getIOObject().getInputStream();
    _known_iStreams.insert(std::make_pair(added_link_id, iStream));
  }

  _readableIOObjectsPool.insert(iStream);
  _usedLinks.insert(std::make_pair(added_link_id, link));
  smsc_log_info(_logger, "PacketReader::_addInputStream::: input stream for link with id='%s' has been added",
                added_link_id.toString().c_str());
}

void
PacketReader::_removeInputStream(const LinkId& removed_link_id)
{
  corex::io::InputStream* iStream;
  {
    smsc::core::synchronization::MutexGuard synchronize(_lockForKnownIStreams);

    istream_registry_t::iterator iter = _known_iStreams.find(removed_link_id);
    if ( iter == _known_iStreams.end() )
      return;

    iStream = iter->second;
    _known_iStreams.erase(iter);
  }
  _readableIOObjectsPool.remove(iStream);
  _usedLinks.erase(removed_link_id);
  smsc_log_info(_logger, "PacketReader::_removeInputStream::: input stream for link with id='%s' has been removed",
                removed_link_id.toString().c_str());
}

}}}
