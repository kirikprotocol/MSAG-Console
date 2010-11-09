#ifndef __EYELINE_UTILX_SUBSYSTEM_HPP__
# define __EYELINE_UTILX_SUBSYSTEM_HPP__

# include <string>
# include <eyeline/utilx/runtime_cfg/RuntimeConfig.hpp>

namespace eyeline {
namespace utilx {

class Subsystem {
public:
  virtual ~Subsystem() {}
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void initialize(runtime_cfg::RuntimeConfig& rconfig) = 0;
  virtual void waitForCompletion() = 0;
  virtual const std::string& getName() const = 0;
};

}}

#endif
