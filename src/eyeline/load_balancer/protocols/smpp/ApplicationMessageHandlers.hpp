#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_APPLICATIONMESSAGEHANDLERS_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_PROTOCOLS_SMPPP_APPLICATIONMESSAGEHANDLERS_HPP__

# include "logger/Logger.h"
# include "eyeline/load_balancer/io_subsystem/types.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkId.hpp"
# include "eyeline/load_balancer/io_subsystem/IOProcessor.hpp"
# include "eyeline/load_balancer/io_subsystem/SwitchingTable.hpp"

# include "eyeline/load_balancer/protocols/smpp/SubmitMulti.hpp"
# include "eyeline/load_balancer/protocols/smpp/SubmitMultiResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/SubmitSm.hpp"
# include "eyeline/load_balancer/protocols/smpp/SubmitSmResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/DeliverSm.hpp"
# include "eyeline/load_balancer/protocols/smpp/DeliverSmResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/DataSm.hpp"
# include "eyeline/load_balancer/protocols/smpp/DataSmResp.hpp"
# include "eyeline/load_balancer/protocols/smpp/MessageForwardingHelper.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class ApplicationMessageHandlers : private MessageForwardingHelper {
public:
  ApplicationMessageHandlers()
  : _logger(smsc::logger::Logger::getInstance("smpp"))
  {}

  virtual void handle(const SubmitMulti& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const SubmitMultiResp& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const SubmitSm& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const SubmitSmResp& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const DeliverSm& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const DeliverSmResp& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const DataSm& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);

  virtual void handle(const DataSmResp& message,
                      const io_subsystem::LinkId& src_link_id,
                      io_subsystem::IOProcessor& io_processor);
private:
  smsc::logger::Logger* _logger;
};

}}}}

#endif
