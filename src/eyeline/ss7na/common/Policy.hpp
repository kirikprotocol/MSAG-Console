#ifndef __EYELINE_SS7NA_COMMON_POLICY_HPP__
# define __EYELINE_SS7NA_COMMON_POLICY_HPP__

# include "util/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

enum policy_type_e {
  RoundRobinPolicyValue, DedicatedPolicyValue
};

template <class VALUE>
class Policy {
public:
  virtual ~Policy() {}
  virtual const VALUE& getValue() = 0;
  virtual policy_type_e getPolicyType() const = 0;
};

template <class VALUE, unsigned MAX_COUNT=16>
class RoundRobinPolicy : public Policy<VALUE> {
public:
  RoundRobinPolicy()
  : _currentCount(0),  _currentIdx(0)
  {}
  void addValue(const VALUE& value) {
    if ( _currentCount + 1 > MAX_COUNT )
      throw smsc::util::Exception("RoundRobinPolicy::addValue::: max. num. of elements = %d exceeded",
                                  MAX_COUNT);
    _values[_currentCount++] = value;
  }
  virtual const VALUE& getValue() {
    if ( !_currentCount )
      throw smsc::util::Exception("RoundRobinPolicy::getValue::: empty set of values");

    if ( _currentIdx == _currentCount ) {
      _currentIdx = 0;
      return _values[_currentIdx++];
    } else
      return _values[_currentIdx++];
  }

  virtual policy_type_e getPolicyType() const { return RoundRobinPolicyValue; }

protected:
  unsigned getMaxCount() const { return MAX_COUNT; }

  VALUE _values[MAX_COUNT];
  unsigned _currentCount, _currentIdx;
};

template <class VALUE>
class DedicatedPolicy : public Policy<VALUE> {
public:
  void setValue(const VALUE& value) {
    _value = value;
  }
  virtual const VALUE& getValue() {
    return _value;
  }

  virtual policy_type_e getPolicyType() const { return DedicatedPolicyValue; }

private:
  VALUE _value;
};

}}}

#endif
