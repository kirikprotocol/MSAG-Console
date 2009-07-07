#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSORMGRIFACE_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_IOPROCESSORMGRIFACE_HPP__

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class IOProcessorMgrIface {
public:
  virtual ~IOProcessorMgrIface() {}

  virtual unsigned getId() const = 0;

  virtual void shutdown() = 0;

  virtual IOProcessor* getVacantIOProcessor() = 0;
};

}}}

#endif
