#ifndef __EYELINE_UTILX_PROTFSM_TCPINDICATIONPRIMITIVE_HPP__
# define __EYELINE_UTILX_PROTFSM_TCPINDICATIONPRIMITIVE_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace utilx {
namespace prot_fsm {

class TCPIndicationPrimitive {
public:
  virtual ~TCPIndicationPrimitive() {}

  virtual uint32_t getIndicationTypeValue() const = 0;

  virtual std::string toString() const = 0 ;
};

}}}

#endif
