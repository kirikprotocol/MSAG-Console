#ifndef __SMSC_UTIL_COMM_COMP_SERIALIZATIONIFACE_HPP__
# define __SMSC_UTIL_COMM_COMP_SERIALIZATIONIFACE_HPP__ 1 

# include <sys/types.h>
# include <vector>

namespace smsc {
namespace util {
namespace comm_comp {

class SerializerIface {
public:
  virtual void serialize(std::vector<uint8_t>& objectBuffer) const = 0;
};

class DeserializerIface {
public:
  virtual void deserialize(const std::vector<uint8_t>& objectBuffer) = 0;
};

}}}

#endif
