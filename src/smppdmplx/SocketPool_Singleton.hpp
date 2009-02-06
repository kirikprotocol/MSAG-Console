#ifndef __SMPPDMPLX_SOCKETPOOL_SINGLETON_HPP__
# define __SMPPDMPLX_SOCKETPOOL_SINGLETON_HPP__

# include <util/Singleton.hpp>
# include <smppdmplx/core_ax/network/SocketPool.hpp>

namespace smpp_dmplx {

class SocketPool_Singleton : public smsc::core_ax::network::SocketPool,
                             public smsc::util::Singleton<SocketPool_Singleton> {};

}
#endif
