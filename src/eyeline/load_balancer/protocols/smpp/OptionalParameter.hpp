#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_OPTIONALPARAMETER_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_OPTIONALPARAMETER_HPP__

# include <sys/types.h>
# include <string>

# include "eyeline/load_balancer/io_subsystem/Packet.hpp"
# include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

template<class T, uint16_t TAG>
class OptionalParameter {
public:
  size_t serialize(io_subsystem::Packet* packet) const;
  size_t deserialize(const io_subsystem::Packet* packet, size_t offset);

  uint16_t getTag() const;
  T getValue() const;
  bool isSetValue() const;
  void setValue(T value);
  uint32_t getParameterSize() const;
private:
  T _value;
  bool _isSetValue;
  enum { _LEN_SIZE = sizeof(uint16_t), _MINIMAL_PARAMETER_LENGTH = sizeof(TAG) + _LEN_SIZE + sizeof(T) };
};

# include <eyeline/load_balancer/protocols/smpp/OptionalParameter_impl.hpp>

}}}}

#endif
