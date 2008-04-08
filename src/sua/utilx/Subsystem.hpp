#ifndef __SUA_UTILS_SUBSYSTEM_HPP__
# define __SUA_UTILS_SUBSYSTEM_HPP__ 1

# include <string>
# include <util/config/ConfigView.h>
# include <sua/sua_layer/runtime_cfg/RuntimeConfig.hpp>

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

}

#endif
