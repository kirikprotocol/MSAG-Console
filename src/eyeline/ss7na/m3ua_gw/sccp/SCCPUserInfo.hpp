#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCCPUSERINFO_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCCPUSERINFO_HPP__

# include <sys/types.h>
# include <string>
# include "eyeline/ss7na/common/LinkId.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class SCCPUserInfo {
public:
  SCCPUserInfo(const std::string& app_id, const common::LinkId& link_id,
               const uint8_t* serviceable_ssn_list,
               uint8_t ssn_list_sz)
  : _appId(app_id), _linkId(link_id)
  {
    memcpy(_ssnList, serviceable_ssn_list, ssn_list_sz);
    memset(_ssnList+ssn_list_sz, 0, sizeof(_ssnList) - ssn_list_sz);
  }

  const std::string& getAppId() const {
    return _appId;
  }

  const common::LinkId& getLinkId() const {
    return _linkId;
  }

  bool isSsnServiceable(uint8_t ssn) const {
    return _ssnList[ssn] != 0;
  }

private:
  std::string _appId;
  common::LinkId _linkId;
  uint8_t _ssnList[256];
};

}}}}

#endif
