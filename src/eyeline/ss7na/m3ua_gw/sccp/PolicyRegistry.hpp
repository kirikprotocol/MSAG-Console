#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_POLICYREGISTRY_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_POLICYREGISTRY_HPP__

# include <sys/types.h>
# include <string>
# include "eyeline/ss7na/m3ua_gw/types.hpp"
# include "eyeline/utilx/ObjectRegistry.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/MTP3SapSelectPolicy.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

template <class POLICY>
class PolicyRegistry : public utilx::Singleton<PolicyRegistry<POLICY> >,
                       public utilx::RefObjectRegistry<POLICY, std::string,
                                                       smsc::core::synchronization::Mutex>
{};

typedef smsc::core::buffers::RefPtr<MTP3SapSelectPolicy, smsc::core::synchronization::Mutex> MTP3SapSelectPolicyRefPtr;
typedef smsc::core::buffers::RefPtr<SCCPUserSelectPolicy, smsc::core::synchronization::Mutex> SCCPUserSelectPolicyRefPtr;

}}}}
#endif
