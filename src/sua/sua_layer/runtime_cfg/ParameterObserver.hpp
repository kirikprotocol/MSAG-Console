#ifndef __PARAMETEROBSERVER_HPP__
# define __PARAMETEROBSERVER_HPP__ 1

# include <sua/sua_layer/runtime_cfg/Parameter.hpp>

namespace runtime_cfg {

class ParameterObserver {
public:
  virtual ~ParameterObserver() {}

  virtual void handle(const Parameter& modifiedParameter) = 0;
};

}

#endif
