#ifndef _SMSC_UTIL_TYPEINFO_H
#define _SMSC_UTIL_TYPEINFO_H

#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace util {

class TypeInfoFactory {
public:
    static bool setTypeVal( int& val );
    static bool setMagicVal( int val, int& magic );
private:
    static smsc::core::synchronization::Mutex typeMutex;
    static int typeVal;
};

/// a template for type infos
template < typename T > struct MagicTICheck;
template < typename T > struct TypeInfo 
{
    friend struct MagicTICheck< T >;

    inline static int typeValue() {
        if ( !val_ && TypeInfoFactory::setTypeVal(val_) ) {
            fprintf(stderr,"type value for '%s' is %x\n",typeid(T).name(),val_);
        }
        return val_;
    }
    inline static int magicValue() {
        if (!magic_ && TypeInfoFactory::setMagicVal(typeValue(),magic_)) {
            fprintf(stderr,"magic value for '%s' is %x\n",typeid(T).name(),magic_);
        }
        return magic_;
    }
private:
    static int val_;
    static int magic_;
};

template < typename T > int TypeInfo< T >::val_ = 0;
template < typename T > int TypeInfo< T >::magic_ = 0;

/// type info checker which is useful for double-deletion
/// and memory corruption checking.
template < typename T > struct MagicTICheck
{
    inline MagicTICheck() : timagic_(TypeInfo<T>::magicValue()) {}
    inline ~MagicTICheck() {
        check();
        timagic_ = 0;
    }
    MagicTICheck( const MagicTICheck& m ) : timagic_(TypeInfo<T>::magicValue()) {
        if ( timagic_ != m.timagic_ ) {
            fprintf(stderr,"initing memory (copy ctor) at %p (%s) with wrong magic=%x from %p, must be %x\n",
                    this, typeid(T).name(), m.timagic_, &m, timagic_ );
            abort();
        }
    }
    MagicTICheck& operator = ( const MagicTICheck& m ) {
        check();
        if (&m == this) return *this;
        if ( timagic_ != m.timagic_ ) {
            fprintf(stderr,"overwriting (assignment) memory at %p (%s) with wrong magic=%x from %p, must be %x!\n",
                    this, typeid(T).name(), m.timagic_, &m, timagic_ );
            abort();
        }
        return *this;
    }
    void check() const {
        if ( timagic_ == TypeInfo< T >::magic_ ) return;
        if ( timagic_ == 0 ) {
            // already deleted!
            fprintf(stderr,"memory at %p (%s) is under double-deletion, magic=0 must be %x!\n",
                    this, typeid(T).name(), TypeInfo<T>::magic_);
            abort();
        }
        // memory corrupted!
        fprintf(stderr,"memory at %p (%s) is corrupted, magic=%x must be %x!\n",
                this, typeid(T).name(), timagic_, TypeInfo<T>::magic_);
        abort();
    }

private:
    int timagic_;
};

/// macros to ease typecheck
#ifdef MAGICTYPECHECK
#define DECLMAGTC(x) smsc::util::MagicTICheck< x >  magicticheck_
#define CHECKMAGTC   magicticheck_.check()
#else
#define DECLMAGTC(x) static void magicticheck_()
#define CHECKMAGTC
#endif

}
}

#endif
