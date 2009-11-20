#ifndef _SMSC_UTIL_PTRDESTROY_H
#define _SMSC_UTIL_PTRDESTROY_H

namespace smsc {
namespace util {

struct PtrDestroy
{
    template < class T > void operator () ( const T* t ) const
    {
        delete t;
    }
};

}
}

#endif /* !_SCAG_UTIL_PTRDESTROY_H */
