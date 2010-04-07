#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MTP3SAPINFO_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MTP3SAPINFO_HPP__

# include <sys/types.h>
# include "eyeline/ss7na/common/types.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class MTP3SapInfo {
public:
  MTP3SapInfo(common::point_code_t lpc=0, common::point_code_t dpc=0)
  : _lpc(lpc), _dpc(dpc)
  {}

  common::point_code_t getLPC() const { return _lpc; }
  common::point_code_t getDPC() const { return _dpc; }

  bool operator== (const MTP3SapInfo& rhs) {
    if ( _lpc == rhs._lpc && _dpc == rhs._dpc )
      return true;
    return false;
  }

  bool operator!= (const MTP3SapInfo& rhs) {
    if ( _lpc != rhs._lpc || _dpc != rhs._dpc )
      return true;
    return false;
  }
private:
  common::point_code_t _lpc, _dpc;
};

}}}}

#endif
