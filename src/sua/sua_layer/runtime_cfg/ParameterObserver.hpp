#ifndef __PARAMETEROBSERVER_HPP__
# define __PARAMETEROBSERVER_HPP__ 1

# include <sua/sua_layer/runtime_cfg/Parameter.hpp>

namespace runtime_cfg {

class ParameterObserver {
public:
  virtual ~ParameterObserver() {}

  virtual void addParameterEventHandler(const CompositeParameter& context, Parameter* addedParameter) {}
  // return added CompositeParameter
  virtual CompositeParameter* addParameterEventHandler(const CompositeParameter& context, CompositeParameter* addedParameter) { return NULL; }
  // return added CompositeParameter
  virtual void addParameterEventHandler(CompositeParameter* context, Parameter* addedParameter) {}

  virtual void changeParameterEventHandler(const Parameter& modifiedParameter) {}
  virtual void changeParameterEventHandler(const CompositeParameter& context, const Parameter& modifiedParameter) {}
  virtual void changeParameterEventHandler(CompositeParameter* context, const Parameter& modifiedParameter) {}

  virtual void removeParameterEventHandler(const Parameter& removedParameter) {}
  virtual void removeParameterEventHandler(const CompositeParameter& context, const Parameter& removedParameter) {}
};

}

#endif
