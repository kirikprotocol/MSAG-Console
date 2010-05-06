#ifndef __EYELINE_UTILX_RUNTIMECFG_COMPOSITEPARAMETER_HPP__
# define __EYELINE_UTILX_RUNTIMECFG_COMPOSITEPARAMETER_HPP__

# include <stdio.h>
# include <map>
# include "eyeline/utilx/runtime_cfg/Parameter.hpp"

namespace eyeline {
namespace utilx {
namespace runtime_cfg {

class CompositeParameter : public Parameter {
  typedef std::multimap<std::string, Parameter*> parameters_t;
  typedef std::multimap<std::string, CompositeParameter*> composite_parameters_t;
public:
  CompositeParameter(const std::string& name);
  
  CompositeParameter(const std::string& name, const std::string& value);

  virtual ~CompositeParameter();

  virtual bool isComposite() const;

  void addParameter(Parameter* parameter);
  CompositeParameter* addParameter(CompositeParameter* parameter);

  template <class PARAMETER_TYPE>
  PARAMETER_TYPE* getParameter(const std::string& param_mame);

  template <class PARAMETER_TYPE>
  const PARAMETER_TYPE* getParameter(const std::string& param_name) const;

  typedef composite_parameters_t::iterator iterator_type_t;
  typedef composite_parameters_t container_type_t;

  template <class PARAMETER_TYPE>
  class Iterator {
  public:
    bool hasElement() const { return _begin != _end; }
    void next() { ++_begin; }
    PARAMETER_TYPE* getCurrentElement() { return _begin->second; }
    const PARAMETER_TYPE* getCurrentElement() const { return _begin->second; }
    void removeElement() {
      delete _begin->second; _container.erase(_begin++);
    }
  private:
    Iterator(typename PARAMETER_TYPE::iterator_type_t& begin,
             typename PARAMETER_TYPE::iterator_type_t& end,
             typename PARAMETER_TYPE::container_type_t& container)
      : _begin(begin), _end(end), _container(container) {}

    friend class CompositeParameter;
    typename PARAMETER_TYPE::iterator_type_t _begin, _end;
    typename PARAMETER_TYPE::container_type_t& _container;
  };

  template <class PARAMETER_TYPE>
  Iterator<PARAMETER_TYPE> getIterator(const std::string& parameter_name);

  std::string printParamaterValue() const;
private:
  parameters_t _parameters;
  composite_parameters_t _compositeParameters;
  CompositeParameter(const CompositeParameter& rhs);
  CompositeParameter& operator=(const CompositeParameter& rhs);
};

# include <eyeline/utilx/runtime_cfg/CompositeParameter_impl.hpp>

}}}

#endif
