#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKETWRITER_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKETWRITER_HPP__

# include <map>

# include "logger/Logger.h"
# include "core/threads/Thread.hpp"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/corex/io/IOObjectsPool.hpp"
# include "eyeline/corex/io/Pipe.hpp"
# include "eyeline/utilx/FakeMutex.hpp"
# include "eyeline/utilx/ArrayBasedQueue.hpp"

# include "eyeline/load_balancer/io_subsystem/types.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/Packet.hpp"
# include "eyeline/load_balancer/io_subsystem/PacketsCache.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor;

class PacketWriter : public smsc::core::threads::Thread {
public:
  PacketWriter(IOProcessor& io_processor, size_t max_out_packets_queue_size,
               unsigned max_streams_per_writer);

  virtual int Execute();
  void shutdown();
  void addOutputStream(const LinkId& link_id);
  void removeOutputStream(const LinkId& link_id);
  void send(const Packet* packet, const LinkId& link_id);

protected:
  typedef enum { ADD_OSTREAM=0, REMOVE_OSTREAM, PUT_PACKET, SHUTDOWN, UNKNOWN } task_type_t;
  struct TaskType {
    TaskType()
    : taskType(UNKNOWN)
    {}

    TaskType(task_type_t task_type, const LinkId& link_id=LinkId())
    : taskType(task_type), linkId(link_id)
    {}
    task_type_t taskType;
    LinkId linkId;
  };

  void scheduleTask(const TaskType& task);
  void _addOutputStream(const LinkId& link_id);
  void _removeOutputStream(const LinkId& link_id);

  void putPacketToQueue(const Packet* packet, const LinkId& link_id);
  bool processNewTask(const Packet** packet, LinkId* link_id);
  corex::io::OutputStream* getOutputStream(const LinkId& link_id);
  void processSignallingStream();
  void processNewPacketForWrite(const Packet* packet,
                                const LinkId& link_id);
  void processReadyOutputStream(corex::io::OutputStream* o_stream);

private:
  smsc::logger::Logger* _logger;
  IOProcessor& _ioProcessor;
  bool _shutdownInProgress;

  corex::io::UnnamedPipe _pipe;
  typedef corex::io::IOObjectsPool_tmpl<utilx::FakeMutex> IOObjectsPool_notMTSafe;
  IOObjectsPool_notMTSafe _signallingAndWritableObjectsPool;

  corex::io::OutputStream* _signallingEndPointForNewPacketPublishing;
  corex::io::InputStream* _signallingEndPointForGotNewPacketEvent;

  typedef std::pair<const Packet*, LinkId> queue_element_t;
  utilx::ArrayBasedQueue<queue_element_t> _packetsQueue;

  typedef std::map<LinkId, corex::io::OutputStream*> ostream_registry_t;
  ostream_registry_t _known_oStreams;
  smsc::core::synchronization::Mutex _lockForKnownOStreams;
  PacketsCache _packetsInCache;

  utilx::ArrayBasedQueue<TaskType> _newSchedTasks;

  typedef std::map<LinkId, LinkRefPtr> used_links_t;
  used_links_t _usedLinks;
};

}}}

#endif
