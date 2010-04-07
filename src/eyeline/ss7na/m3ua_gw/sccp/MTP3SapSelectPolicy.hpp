#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MTP3SAPSELECTPOLICY_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MTP3SAPSELECTPOLICY_HPP__

# include <string>

# include "eyeline/ss7na/common/Policy.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/MTP3SapInfo.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/SCCPUserInfo.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class MTP3SapSelectPolicy : public common::Policy<MTP3SapInfo> {
public:
  using common::Policy<MTP3SapInfo>::getValue;
  virtual const MTP3SapInfo& getValue(uint32_t sequence_control) = 0;
};

class MTP3SapRoundRobinPolicy : public MTP3SapSelectPolicy,
                                public common::RoundRobinPolicy<MTP3SapInfo> {
public:
  virtual const MTP3SapInfo& getValue() {
    return common::RoundRobinPolicy<MTP3SapInfo>::getValue();
  }

  virtual const MTP3SapInfo& getValue(uint32_t sequence_control) {
    unsigned idx = sequence_control % _currentCount;
    return _values[idx];
  }

  virtual common::policy_type_e getPolicyType() const {
    return common::RoundRobinPolicy<MTP3SapInfo>::getPolicyType();
  }
};

class MTP3SapDedicatedPolicy : public MTP3SapSelectPolicy,
                               public common::DedicatedPolicy<MTP3SapInfo> {
public:
  virtual const MTP3SapInfo& getValue() {
    return common::DedicatedPolicy<MTP3SapInfo>::getValue();
  }

  virtual const MTP3SapInfo& getValue(uint32_t sequence_control) {
    return common::DedicatedPolicy<MTP3SapInfo>::getValue();
  }

  virtual common::policy_type_e getPolicyType() const {
    return common::DedicatedPolicy<MTP3SapInfo>::getPolicyType();
  }
};

typedef common::Policy<std::string /*appId*/> SCCPUserSelectPolicy;
typedef common::RoundRobinPolicy<std::string /*appId*/> SCCPUserRoundRobinPolicy;
typedef common::DedicatedPolicy<std::string /*appId*/> SCCPUserDedicatedPolicy;


}}}}
#endif
