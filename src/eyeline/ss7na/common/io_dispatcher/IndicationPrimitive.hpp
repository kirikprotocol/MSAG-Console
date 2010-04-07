#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_INDICATIONPRIMITIVE_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_INDICATIONPRIMITIVE_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class IndicationPrimitive {
public:
  virtual ~IndicationPrimitive() {}

  virtual uint32_t getIndicationTypeValue() const = 0;

  virtual std::string toString() const = 0;
};

}}}}

#endif
