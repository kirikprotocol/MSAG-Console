#ifndef __EYELINE_UTILX_PROTFSM_SCTPESTABLISHIND_HPP__
# define __EYELINE_UTILX_PROTFSM_SCTPESTABLISHIND_HPP__

# include <eyeline/utilx/prot_fsm/SCTPIndicationPrimitive.hpp>

namespace eyeline {
namespace utilx {
namespace prot_fsm {

class SctpEstablishInd : public SCTPIndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const;

  virtual std::string toString() const;

  static const uint32_t SCTP_ESTABLISH_IND = 0xFF000003;
};

}}}

#endif
