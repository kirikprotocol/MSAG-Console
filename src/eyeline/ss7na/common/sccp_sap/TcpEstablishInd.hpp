#ifndef __EYELINE_SS7NA_COMMON_SCCPSAP_TCPESTABLISHIND_HPP__
# define __EYELINE_SS7NA_COMMON_SCCPSAP_TCPESTABLISHIND_HPP__

# include "eyeline/ss7na/common/io_dispatcher/IndicationPrimitive.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

class TcpEstablishInd : public io_dispatcher::IndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const { return TCP_ESTABLISH_IND; }

  virtual std::string toString() const { return "TCP_ESTABLISH_IND"; }
private:
  static const uint32_t TCP_ESTABLISH_IND=0xFF000001;
};

}}}}

#endif
