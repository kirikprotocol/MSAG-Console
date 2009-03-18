#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_INDICATIONPRIMITIVE_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_INDICATIONPRIMITIVE_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

class IndicationPrimitive {
public:
  virtual ~IndicationPrimitive() {}

  virtual uint32_t getIndicationTypeValue() const = 0;

  virtual std::string toString() const = 0;
};

}}}}

#endif
