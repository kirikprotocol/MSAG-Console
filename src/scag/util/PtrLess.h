#ifndef _SCAG_UTIL_PTRLESS_H
#define _SCAG_UTIL_PTRLESS_H

class PtrLess
{
public:
    template < class T > bool operator () ( const T* t1, const T* t2 ) const 
    {
        return (*t1 < *t2);
    }
};

#endif /* !_SCAG_UTIL_PTRLESS_H */
