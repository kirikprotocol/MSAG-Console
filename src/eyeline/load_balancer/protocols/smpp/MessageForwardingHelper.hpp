#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_MESSAGEFORWARDINGHELPER_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_MESSAGEFORWARDINGHELPER_HPP__

# include "eyeline/load_balancer/io_subsystem/types.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"
# include "eyeline/load_balancer/io_subsystem/SwitchingTable.hpp"

# include "eyeline/load_balancer/protocols/smpp/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class MessageForwardingHelper {
protected:

  template <class MESSAGE>
  void
  forwardMessageToSmsc(const MESSAGE& message,
                       const io_subsystem::LinkId& src_link_id,
                       io_subsystem::IOProcessor& io_processor);

  template <class MESSAGE>
  void
  forwardMultipartMessageToSmsc(const MESSAGE& message,
                                const io_subsystem::LinkId& src_link_id,
                                io_subsystem::IOProcessor& io_processor);

  template <class MESSAGE>
  io_subsystem::LinkId
  forwardMessageToSme(const MESSAGE& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  template <class MESSAGE>
  void
  forwardSmppResponseToSmsc(const MESSAGE& message,
                            const io_subsystem::LinkId& src_link_id,
                            io_subsystem::IOProcessor& io_processor);

  template <class MESSAGE>
  void
  forwardServiceRequestToConcreteSmsc(const MESSAGE& message,
                                      const io_subsystem::LinkId& src_link_id,
                                      io_subsystem::IOProcessor& io_processor);
};

# include "eyeline/load_balancer/protocols/smpp/MessageForwardingHelper_impl.hpp"

}}}}

#endif
