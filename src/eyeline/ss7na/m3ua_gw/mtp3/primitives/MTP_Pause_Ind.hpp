#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_PRIMITIVES_MTPPAUSEIND_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_PRIMITIVES_MTPPAUSEIND_HPP__

# include <string>
# include "eyeline/ss7na/common/PointCode.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace primitives {

class MTP_Pause_Ind {
public:
  explicit MTP_Pause_Ind(common::point_code_t pc)
  : _pc(pc)
  {}

  common::point_code_t getAffectedDPC() const {
    return _pc;
  }

  std::string toString() const {
    char strBuf[256];
    sprintf(strBuf, "affectedDPC=%u", _pc);
    return strBuf;
  }

private:
  common::point_code_t _pc;
};

}}}}}

#endif
