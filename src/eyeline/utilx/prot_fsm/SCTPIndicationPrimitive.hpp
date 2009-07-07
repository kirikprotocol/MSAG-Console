#ifndef __EYELINE_UTILX_PROTFSM_SCTPINDICATIONPRIMITIVE_HPP__
# define __EYELINE_UTILX_PROTFSM_SCTPINDICATIONPRIMITIVE_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace utilx {
namespace prot_fsm {

class SCTPIndicationPrimitive {
public:
  virtual ~SCTPIndicationPrimitive() {}

  virtual uint32_t getIndicationTypeValue() const = 0;

  virtual std::string toString() const = 0 ;
};

}}}

#endif
