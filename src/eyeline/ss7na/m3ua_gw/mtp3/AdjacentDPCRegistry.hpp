#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_ADJACENTDPCREGISTRY_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_ADJACENTDPCREGISTRY_HPP__

# include "eyeline/utilx/ObjectRegistry.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {

class AdjacentDPCRegistry : public utilx::Singleton<AdjacentDPCRegistry>,
                            public utilx::ObjectRegistry<common::point_code_t,
                                                         common::LinkId,
                                                         smsc::core::synchronization::Mutex>
{};

}}}}

#endif
