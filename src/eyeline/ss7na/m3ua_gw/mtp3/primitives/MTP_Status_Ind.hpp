#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_PRIMITIVES_MTPSTATUSIND_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_PRIMITIVES_MTPSTATUSIND_HPP__

# include "eyeline/ss7na/m3ua_gw/types.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace primitives {

class MTP_Status_Ind {
public:
  MTP_Status_Ind(common::point_code_t pc, mtp_status_cause_e cause)
  : _pc(pc), _cause(cause)
  {}

  common::point_code_t getAffectedDPC() const {
    return _pc;
  }

  mtp_status_cause_e getCause() const {
    return _cause;
  }

  std::string toString() const {
    char strBuf[256];
    sprintf(strBuf, "affectedDPC=%u,cause=%u", _pc, _cause);
    return strBuf;
  }

private:
  common::point_code_t _pc;
  mtp_status_cause_e _cause;
};

}}}}}

#endif
