#ifndef __EYELINE_UTILX_PROTFSM_TCPRELEASEIND_HPP__
# define __EYELINE_UTILX_PROTFSM_TCPRELEASEIND_HPP__

# include <eyeline/utilx/prot_fsm/TCPIndicationPrimitive.hpp>

namespace eyeline {
namespace utilx {
namespace prot_fsm {

class TcpReleaseInd : public TCPIndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const;

  virtual std::string toString() const;

  static const uint32_t TCP_RELEASE_IND = 0xFF000002;
};

}}}

#endif
