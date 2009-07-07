#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKETREADER_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PACKETREADER_HPP__

# include <assert.h>
# include <sys/types.h>
# include <list>
# include <map>

# include "logger/Logger.h"
# include "core/threads/Thread.hpp"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/corex/io/IOStreams.hpp"
# include "eyeline/corex/io/IOObjectsPool.hpp"
# include "eyeline/corex/io/Pipe.hpp"
# include "eyeline/utilx/FakeMutex.hpp"

# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/MessagePublisher.hpp"
# include "eyeline/load_balancer/io_subsystem/IdleLinksMonitor.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor;

class PacketReader : public smsc::core::threads::Thread {
public:
  PacketReader(IOProcessor& io_processor,
               MessagePublisher& message_publisher,
               unsigned max_streams_per_reader);

  virtual int Execute();
  void shutdown();

  void addInputStream(const LinkId& link_id);
  void removeInputStream(const LinkId& link_id);

protected:
  typedef enum { ADD_ISTREAM=1, REMOVE_ISTREAM=2, SHUTDOWN, UNKNOWN } task_type_t;
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
  void receivePacketAndPublishIt(corex::io::InputStream* i_stream);

  void cleanUpBrokenConnection(Link* link);

  void processInputStreamManipulationTask();
  void _addInputStream(const LinkId& added_link_id);
  void _removeInputStream(const LinkId& remove_iStream_task);

private:
  smsc::logger::Logger* _logger;
  IOProcessor& _ioProcessor;
  bool _shutdownInProgress;
  MessagePublisher& _messagePublisher;

  typedef corex::io::IOObjectsPool_tmpl<utilx::FakeMutex> IOObjectsPool_notMTSafe;
  IOObjectsPool_notMTSafe _readableIOObjectsPool;

  typedef std::map<LinkId, corex::io::InputStream*> istream_registry_t;
  istream_registry_t _known_iStreams;
  smsc::core::synchronization::Mutex _lockForKnownIStreams;

  utilx::ArrayBasedQueue<TaskType> _newSchedTasks;
  corex::io::UnnamedPipe _pipe;
  corex::io::InputStream* _signallingEndpoint;
  corex::io::OutputStream* _notificationEndpoint;

  typedef std::map<LinkId, LinkRefPtr> used_links_t;
  used_links_t _usedLinks;
};

}}}

#endif
