#ifndef __EYELINE_SS7NA_COMMON_SCCPSAP_TCPRELEASEIND_HPP__
# define __EYELINE_SS7NA_COMMON_SCCPSAP_TCPRELEASEIND_HPP__

# include "eyeline/ss7na/common/io_dispatcher/IndicationPrimitive.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

class TcpReleaseInd : public io_dispatcher::IndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const { return TCP_RELEASE_IND; }

  virtual std::string toString() const { return "TCP_RELEASE_IND"; }
private:
  static const int TCP_RELEASE_IND = 0xFF000002;
};

}}}}

#endif
