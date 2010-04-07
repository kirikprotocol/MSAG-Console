#ifndef __EYELINE_SS7NA_COMMON_SIG_SCTPESTABLISHIND_HPP__
# define __EYELINE_SS7NA_COMMON_SIG_SCTPESTABLISHIND_HPP__

# include "eyeline/ss7na/common/io_dispatcher/IndicationPrimitive.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sig {

class SctpEstablishInd : public io_dispatcher::IndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const { return SCTP_CONNECTION_IND; }

  virtual std::string toString() const {return "SCTP_ESTABLISH_IND"; }

private:
  static const int SCTP_CONNECTION_IND = 0xFF000003;
};

}}}}

#endif
