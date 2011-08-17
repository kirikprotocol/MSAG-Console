#include <assert.h>
#include <utility>
#include "util/Exception.hpp"
#include "core/synchronization/MutexGuard.hpp"

#include "IOProcessor.hpp"
#include "PacketWriter.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

PacketWriter::PacketWriter(IOProcessor& io_processor,
                           size_t max_out_packets_queue_size,
                           unsigned max_streams_per_writer)
  : _logger(smsc::logger::Logger::getInstance("io_subsystem")),
    _ioProcessor(io_processor), _shutdownInProgress(false),
    _signallingAndWritableObjectsPool(max_streams_per_writer+1),
    _packetsQueue(max_out_packets_queue_size),
    _newSchedTasks(max_streams_per_writer)
{
  _signallingEndPointForNewPacketPublishing = _pipe.getLeftSide()->getOutputStream();
  _signallingEndPointForGotNewPacketEvent = _pipe.getRightSide()->getInputStream();
  _signallingAndWritableObjectsPool.insert(_signallingEndPointForGotNewPacketEvent);
}

int
PacketWriter::Execute()
{
  smsc_log_debug(_logger, "PacketWriter::Execute::: thread stared");
  while (!_shutdownInProgress) {
    try {
      int listenStatus = _signallingAndWritableObjectsPool.listen();
      smsc_log_debug(_logger, "PacketWriter::Execute::: listenStatus=%d", listenStatus);

      if ( listenStatus & IOObjectsPool_notMTSafe::OK_WRITE_READY ) {
        corex::io::OutputStream* oStream = _signallingAndWritableObjectsPool.getNextReadyOutputStream();
        smsc_log_debug(_logger, "PacketWriter::Execute::: bypass writable sockets");

        while ( oStream ) {
          processReadyOutputStream(oStream);
          oStream = _signallingAndWritableObjectsPool.getNextReadyOutputStream();
        }
      }

      if ( listenStatus & IOObjectsPool_notMTSafe::OK_READ_READY )
        processSignallingStream();

    } catch (std::exception& ex) {
      smsc_log_debug(_logger, "PacketWriter::Execute::: caught exception [%s]", ex.what());
    }
  }

  return 0;
}

void
PacketWriter::shutdown()
{
  _shutdownInProgress = true;
  scheduleTask(TaskType(SHUTDOWN));
}

void
PacketWriter::addOutputStream(const LinkId& link_id)
{
  {
    smsc::core::synchronization::MutexGuard synchronize(_lockForKnownOStreams);
    if ( _known_oStreams.find(link_id) !=  _known_oStreams.end() )
      return;
  }
  scheduleTask(TaskType(ADD_OSTREAM, link_id));
}

void
PacketWriter::removeOutputStream(const LinkId& link_id)
{
  {
    smsc::core::synchronization::MutexGuard synchronize(_lockForKnownOStreams);
    ostream_registry_t::iterator iter = _known_oStreams.find(link_id);
    if ( iter == _known_oStreams.end())
      return;
  }
  smsc_log_debug(_logger, "PacketWriter::removeOutputStream::: try remove output stream for link with id='%s'",
                 link_id.toString().c_str());
  scheduleTask(TaskType(REMOVE_OSTREAM, link_id));
}

void
PacketWriter::scheduleTask(const TaskType& task)
{
  _newSchedTasks.enqueue(task);

  uint8_t event=task.taskType;
  try {
    _signallingEndPointForNewPacketPublishing->write(&event, sizeof(event));
  } catch(std::exception& ex) {
    smsc_log_error(_logger, "PacketWriter::scheduleTask::: caught exception '%s'", ex.what());
  }
}

void
PacketWriter::_addOutputStream(const LinkId& link_id)
{
  LinkRefPtr link = _ioProcessor.getLink(link_id);

  if ( !link.Get() ) {
    smsc_log_error(_logger, "PacketWriter::_addOutputStream::: can't add output stream for non-existent link with id='%s'",
                   link_id.toString().c_str());
    return;
  }

  {
    smsc::core::synchronization::MutexGuard synchronize(_lockForKnownOStreams);
    if ( _known_oStreams.find(link_id) != _known_oStreams.end() )
      return;
    _known_oStreams.insert(std::make_pair(link_id, link->getIOObject().getOutputStream()));
  }

  _usedLinks.insert(std::make_pair(link_id, link));
  smsc_log_info(_logger, "PacketWriter::_addOutputStream::: output stream for link with id='%s' has been added",
                 link_id.toString().c_str());
}

void
PacketWriter::_removeOutputStream(const LinkId& link_id)
{
  corex::io::OutputStream* oStream;
  {
    smsc::core::synchronization::MutexGuard synchronize(_lockForKnownOStreams);
    ostream_registry_t::iterator iter = _known_oStreams.find(link_id);
    if ( iter == _known_oStreams.end())
      return;
    oStream = iter->second;
    _known_oStreams.erase(iter);
  }
  _packetsInCache.removePacketsFromCache(oStream);
  _signallingAndWritableObjectsPool.remove(oStream);
  _usedLinks.erase(link_id);
  smsc_log_info(_logger, "PacketWriter::_removeOutputStream::: output stream for link with id='%s' has been removed",
                link_id.toString().c_str());
}

void
PacketWriter::send(const Packet* packet, const LinkId& link_id)
{
  putPacketToQueue(packet, link_id);
}

void
PacketWriter::putPacketToQueue(const Packet* packet, const LinkId& link_id)
{
  uint8_t event=PUT_PACKET;
  smsc_log_debug(_logger, "PacketWriter::putPacketToQueue::: schedule packet (packet_type=0x%08X) for writting, link_id=%s",
                 packet->packet_type, link_id.toString().c_str());
  _packetsQueue.enqueue(std::make_pair(packet, link_id));
  _signallingEndPointForNewPacketPublishing->write(&event, sizeof(event));
}

bool
PacketWriter::processNewTask(const Packet** packet, LinkId* link_id)
{
  uint8_t event;
  _signallingEndPointForGotNewPacketEvent->read(&event, sizeof(event));

  smsc_log_debug(_logger, "PacketWriter::processNewTask::: got event=%d", event);
  assert(event == PUT_PACKET ||
         event == ADD_OSTREAM ||
         event == REMOVE_OSTREAM ||
         event == SHUTDOWN);

  if ( event == PUT_PACKET ) {
    smsc_log_debug(_logger, "PacketWriter::popPacketFromQueue::: extract next scheduled packet");

    queue_element_t q_element = _packetsQueue.dequeue();
    *packet = q_element.first;
    *link_id = q_element.second;
    return true;
  } else {
    const TaskType& task = _newSchedTasks.dequeue();
    smsc_log_debug(_logger, "PacketWriter::processNewTask::: fetched taskType '%d' for link with id='%s'",
                   task.taskType, task.linkId.toString().c_str());
    assert(event == task.taskType);
    if ( task.taskType == ADD_OSTREAM )
      _addOutputStream(task.linkId);
    else if ( task.taskType == REMOVE_OSTREAM )
      _removeOutputStream(task.linkId);
  }
  return false;
}

corex::io::OutputStream*
PacketWriter::getOutputStream(const LinkId& link_id)
{
  smsc::core::synchronization::MutexGuard synchronize(_lockForKnownOStreams);
  ostream_registry_t::iterator iter = _known_oStreams.find(link_id);
  if ( iter == _known_oStreams.end() )
    throw smsc::util::Exception("PacketWriter::getOutputStream::: there isn't OutputStream for linkId=[%s]",link_id.toString().c_str());
  return iter->second;
}

void
PacketWriter::processSignallingStream()
{
  corex::io::InputStream* iStream =
    _signallingAndWritableObjectsPool.getNextReadyInputStream();
  if ( iStream != _signallingEndPointForGotNewPacketEvent )
    throw smsc::util::Exception("PacketWriter::processSignallingStream::: ready InputStream is not a stream dedicated for signalling events");

  const Packet* packet;
  LinkId linkId;
  if ( processNewTask(&packet, &linkId) )
    processNewPacketForWrite(packet, linkId);
}

void
PacketWriter::processNewPacketForWrite(const Packet* packet,
                                       const LinkId& link_id)
{
  smsc_log_debug(_logger, "PacketWriter::processNewPacketForWrite::: got scheduled packet (packet_type=0x%08X) for writing, link_id=%s",
                 packet->packet_type, link_id.toString().c_str());
  corex::io::OutputStream* oStream = getOutputStream(link_id);
  _packetsInCache.storePacketInCache(oStream, packet);
  _signallingAndWritableObjectsPool.insert(oStream);
  smsc_log_debug(_logger, "PacketWriter::processNewPacketForWrite::: packet with type=0x%08X will be written to socket='%s'",
                 packet->packet_type, oStream->getOwner()->toString().c_str());
}

void
PacketWriter::processReadyOutputStream(corex::io::OutputStream* o_stream)
{
  smsc_log_debug(_logger, "PacketWriter::processReadyOutputStream::: next socket [%s] is ready for write",
                 o_stream->getOwner()->toString().c_str());

  PacketsCache::cache_element packetFromCache = _packetsInCache.getPacket(o_stream);
  smsc_log_debug(_logger, "PacketWriter::Execute::: got packet from cache: (packet_type=0x%08X, offset=%d)",
                 packetFromCache.packet->packet_type, packetFromCache.offset);

  ssize_t writtenBytes;
  LinkId linkId(o_stream->getOwner()->getId());
  try {
    writtenBytes = o_stream->write(packetFromCache.packet->packet_data + packetFromCache.offset, packetFromCache.packet->packet_data_len - packetFromCache.offset);
    smsc_log_debug(_logger, "PacketWriter::processReadyOutputStream::: %d bytes has been written to socket=[%s]",
                   writtenBytes, o_stream->getOwner()->toString().c_str());
    if ( _packetsInCache.commitWrittenPacketBytes(o_stream, writtenBytes) )
      _signallingAndWritableObjectsPool.remove(o_stream);
  } catch (corex::io::BrokenPipe& ex) {
    smsc_log_debug(_logger, "PacketWriter::processReadyOutputStream::: caught BrokenPipe exception [%s] for socket [%s]",
                   ex.what(), o_stream->getOwner()->toString().c_str());
    _removeOutputStream(linkId);
  }
}

}}}
