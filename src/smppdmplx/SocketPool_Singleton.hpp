#ifndef __SMPPDMPLX_SOCKETPOOL_SINGLETON_HPP__
# define __SMPPDMPLX_SOCKETPOOL_SINGLETON_HPP__ 1

# include <util/Singleton.hpp>
# include <core_ax/network/SocketPool.hpp>

class SocketPool_Singleton : public smsc::core_ax::network::SocketPool,
                             public smsc::util::Singleton<SocketPool_Singleton> {};

#endif
