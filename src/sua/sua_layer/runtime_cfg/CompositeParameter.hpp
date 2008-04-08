#ifndef COMPOSITEPARAMETER_HPP_HEADER_INCLUDED_B8C78862
#define COMPOSITEPARAMETER_HPP_HEADER_INCLUDED_B8C78862

#include <sua/sua_layer/runtime_cfg/Parameter.hpp>
#include <map>

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
  PARAMETER_TYPE* getParameter(const std::string& parameterName) const;

  typedef composite_parameters_t::const_iterator iterator_trait_t;

  template <class PARAMETER_TYPE>
  class Iterator {
  public:
    bool hasElement() const { return _begin != _end; }
    void next() { ++_begin; }
    PARAMETER_TYPE* getCurrentElement() const { return _begin->second; }
  private:
    Iterator(typename PARAMETER_TYPE::iterator_trait_t& begin, typename PARAMETER_TYPE::iterator_trait_t& end)
      : _begin(begin), _end(end) {}

    friend class CompositeParameter;
    typename PARAMETER_TYPE::iterator_trait_t _begin, _end;
  };

  template <class PARAMETER_TYPE>
  Iterator<PARAMETER_TYPE> getIterator(const std::string& parameterName) const ;
private:
  parameters_t _parameters;
  composite_parameters_t _compositeParameters;
};

# include "sua/sua_layer/runtime_cfg/CompositeParameter_impl.hpp"

}

#endif /* COMPOSITEPARAMETER_HPP_HEADER_INCLUDED_B8C78862 */
