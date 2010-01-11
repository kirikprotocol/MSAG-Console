#ifndef _SMSC_UTIL_TYPEINFO_H
#define _SMSC_UTIL_TYPEINFO_H

#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace util {

class TypeInfoFactory {
public:
    static void getTypeVal( int& val );
private:
    static smsc::core::synchronization::Mutex typeMutex;
    static int typeVal;
};

/// a template for type infos
template < class T > struct TypeInfo 
{
    inline static int typeValue() {
        if ( !val_ ) { TypeInfoFactory::getTypeVal(val_); }
        return val_;
    }
private:
    static int val_;
};

template < class T > int TypeInfo< T >::val_ = 0;

}
}

#endif
