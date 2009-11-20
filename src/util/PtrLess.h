#ifndef _SMSC_UTIL_PTRLESS_H
#define _SMSC_UTIL_PTRLESS_H

namespace smsc {
namespace util {

struct PtrLess
{
    template < class T > bool operator () ( const T* t1, const T* t2 ) const
    {
        return (*t1 < *t2);
    }
};

}
}

#endif /* !_SCAG_UTIL_PTRLESS_H */
