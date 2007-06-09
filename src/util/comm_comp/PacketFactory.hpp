#ifndef __SMSC_UTIL_PACKETFACTORY_HPP__
# define __SMSC_UTIL_PACKETFACTORY_HPP__ 1

# include <util/Singleton.hpp>
# include <util/Factory.hpp>

namespace smsc {
namespace util {
namespace comm_comp {

template <class APPLICATION_PACKET>
class PacketFactory : public smsc::util::Singleton <PacketFactory<APPLICATION_PACKET> >,
                      public smsc::util::FactoryT<uint16_t, APPLICATION_PACKET> {
public:
  PacketFactory();
};

}}}

#endif
