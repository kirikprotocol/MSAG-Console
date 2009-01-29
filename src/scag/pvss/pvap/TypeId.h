#ifndef _SCAG_PVSS_PVAP_TYPEID_H
#define _SCAG_PVSS_PVAP_TYPEID_H

#include "core/synchronization/Mutex.hpp"

namespace scag {
namespace pvss {
namespace pvap {

class TypeIdFactory
{
public:
    static int getId();
};

template < typename T > class TypeId
{
private:
    static smsc::core::synchronization::Mutex mtx;
    static int idValue;

public:
    static int getId() {
        static bool init = false;
        if ( ! init ) {
            smsc::core::synchronization::MutexGuard mg(mtx);
            if ( ! init ) {
                idValue = TypeIdFactory::getId();
                init = true;
            }
        }
        return idValue;
    }
};

template < typename T > smsc::core::synchronization::Mutex TypeId< T >::mtx;
template < typename T > int TypeId< T >::idValue = 0;

} // namespace pvap
} // namespace pvss
} // namespace scag

#endif /* !_SCAG_PVSS_PVAP_TYPEID_H */
