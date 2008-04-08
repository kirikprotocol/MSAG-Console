#ifndef __SUA_SUALAYER_IODISPATCHER_INDICATIONPRIMITIVE_HPP__
# define __SUA_SUALAYER_IODISPATCHER_INDICATIONPRIMITIVE_HPP__ 1

# include <sys/types.h>
# include <string>

namespace io_dispatcher {

class IndicationPrimitive {
public:
  virtual ~IndicationPrimitive() {}

  virtual uint32_t getIndicationTypeValue() const = 0;

  virtual std::string toString() const = 0;
};

}

#endif
