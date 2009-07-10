#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSOR_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSOR_HPP__

# include <sys/types.h>
# include <map>
# include <string>

# include "logger/Logger.h"
# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/load_balancer/io_subsystem/types.hpp"
# include "eyeline/load_balancer/io_subsystem/MessagePublisher.hpp"
# include "eyeline/load_balancer/io_subsystem/Binder.hpp"
# include "eyeline/load_balancer/io_subsystem/Reconnector.hpp"
# include "eyeline/load_balancer/io_subsystem/PacketReader.hpp"
# include "eyeline/load_balancer/io_subsystem/PacketWriter.hpp"

# include "eyeline/load_balancer/io_subsystem/Message.hpp"
# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkSet.hpp"
# include "eyeline/load_balancer/io_subsystem/TimeoutEvent.hpp"
# include "eyeline/load_balancer/io_subsystem/SwitchCircuitController.hpp"
# include "eyeline/load_balancer/io_subsystem/InputEventProcessorsPool.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessor {
public:
  IOProcessor(unsigned io_proc_id, SwitchCircuitController& switch_circuit_ctrl,
              unsigned max_events_queue_sz, unsigned reconnect_attempt_period,
              unsigned max_out_packets_queue_size, unsigned max_num_of_event_processors,
              unsigned max_links_per_processor)
  : _ioProcId(io_proc_id), _logger(smsc::logger::Logger::getInstance("io_subsystem")),
    _switchCircuitCtrl(switch_circuit_ctrl), _messagePublisher(max_events_queue_sz),
    _reconnector(*this, reconnect_attempt_period),
    _packetReader(*this, _messagePublisher, max_links_per_processor),
    _packetWriter(*this, max_out_packets_queue_size, max_links_per_processor),
    _eventProcessorsPool("IOPacketEventProcPool", max_num_of_event_processors, _messagePublisher),
    _numOfActiveLinks(0), _maxLinksPerProcessor(max_links_per_processor)
  {}

  void startup();
  void shutdown();

  LinkId addLink(LinkRefPtr& link);
  LinkId addLinkSet(LinkSetRefPtr& link_set);

  LinkRefPtr getLink(const LinkId& link_id) const;
  LinkSetRefPtr getLinkSet(const LinkId& link_set_id) const;

  virtual void removeIncomingLink(const LinkId& link_id);
  LinkId removeOutcomingLink(const LinkId& link_id, bool no_need_reconnect = false);
  virtual void removeLinkSet(const LinkId& link_set_id);
  void cleanUpConnection(const LinkId& link_id);

  unsigned getId() const {
    return _ioProcId;
  }

  virtual Binder& getBinder() = 0;

  Reconnector& getReconnector() {
    return _reconnector;
  }

  bool canProcessNewSocket() const;
  // return empty LinkId value if this link isn't owned by any linkset
  LinkId getLinkSetIdOwnerForThisLink(const LinkId& link_id);

  void scheduleConnectionForRestoring(const LinkId& linkset_id,
                                      Link* terminated_link);

  virtual TimeoutEvent*
  createBindResponseWaitTimeoutEvent(const LinkId& link_id,
                                     Message* response_message_for_send) = 0;

private:
  void terminateConnection(const LinkId& link_id, LinkRefPtr* terminated_link_ref = NULL);

  unsigned _ioProcId;
  smsc::logger::Logger* _logger;
  SwitchCircuitController& _switchCircuitCtrl;
  MessagePublisher _messagePublisher;
  Reconnector _reconnector;
  PacketReader _packetReader;
  PacketWriter _packetWriter;
  InputEventProcessorsPool _eventProcessorsPool;

  unsigned _numOfActiveLinks, _maxLinksPerProcessor;
  typedef std::map<LinkId, LinkRefPtr> active_links_t;
  active_links_t _activeLinks;
  mutable smsc::core::synchronization::Mutex _activeLinksLock;

  typedef std::map<LinkId, LinkSetRefPtr> active_linksets_t;
  active_linksets_t _activeLinkSets;
  mutable smsc::core::synchronization::Mutex _activeLinkSetsLock;
};

}}}

#endif
