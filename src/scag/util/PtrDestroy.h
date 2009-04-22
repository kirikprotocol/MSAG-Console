#ifndef _SCAG_UTIL_PTRDESTROY_H
#define _SCAG_UTIL_PTRDESTROY_H

class PtrDestroy
{
public:
    template < class T > void operator () ( const T* t ) const
    {
        delete t;
    }
};

#endif /* !_SCAG_UTIL_PTRDESTROY_H */
