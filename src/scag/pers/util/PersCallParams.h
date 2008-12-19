#ifndef _SCAG_PERS_UTIL_PERSCALLPARAMS_H
#define _SCAG_PERS_UTIL_PERSCALLPARAMS_H

#include <memory>
#include "PersCommand.h"

namespace scag2 {
namespace pers {
namespace util {

class PersCall;

class PersCallData
{
public:
    friend class PersCall;

    PersCallData( ProfileType  pt,
                  PersCommand* cmd ) :
    type_(pt), cmd_(cmd), ikey_(0) {}

    inline PersCmd cmdType() const { return cmd_->cmdType(); }
    inline ProfileType getType() const { return type_; }

    inline void setKey( const std::string& key ) { skey_ = key; ikey_ = 0; }
    inline void setKey( int32_t key ) { ikey_ = key; skey_.clear(); }

    inline const char* getStringKey() const { return skey_.c_str(); }
    inline int32_t getIntKey() const { return ikey_; }

    // fill a serial buffer
    int fillSB( SerialBuffer& sb, int32_t serial = 0 );
    int readSB( SerialBuffer& sb );
    inline void storeResults( void* ctx ) { cmd_->storeResults(ctx); }

    inline int status() const { return cmd_->status(); }
    void setStatus( int, const char* what = 0 );
    const std::string& exception() const { return exception_; }

private:
    ProfileType                   type_;
    std::auto_ptr< PersCommand >  cmd_;
    std::string                   skey_;
    int32_t                       ikey_;
    std::string                   exception_;
    PersCall*                     next_;
};


// an interface to a pers call
class PersCall
{
public:
    inline PersCall* next() {
        return data().next_;
    }
    inline void setNext( PersCall* n ) {
        data().next_ = n;
    }
    inline void setStatus( int s, const char* what = 0 ) {
        data().setStatus( s, what );
    }
    inline int status() const {
        return const_cast< PersCall* >(this)->data().status();
    }

    virtual PersCallData& data() = 0;
    virtual void continuePersCall( bool drop ) = 0;
};


}
}
}

#endif /* ! _SCAG_PERS_UTIL_PERSCALLPARAMS_H */
