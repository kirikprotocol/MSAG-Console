#ifndef _SCAG_PERS_UTIL_PERSCALLPARAMS_H
#define _SCAG_PERS_UTIL_PERSCALLPARAMS_H

#include <memory>
#include "PersCommand.h"

namespace scag2 {
namespace pers {
namespace util {

class PersCall;
class PersClient;


class PersCallInitiator
{
public:
    virtual void continuePersCall( PersCall* call, bool drop ) = 0;
};


class PersCall
{
public:
    friend class PersClient;

    PersCall( ProfileType  pt,
              PersCommand* cmd,
              void*        ctx ) :
    type_(pt), cmd_(cmd), ikey_(0), next_(0), context_(ctx), initiator_(0) {}

    inline void setKey( const std::string& key ) { skey_ = key; ikey_ = 0; }
    inline void setKey( int32_t key ) { ikey_ = key; skey_.clear(); }

    inline PersCmd cmdType() const { return cmd_->cmdType(); }
    inline ProfileType getType() const { return type_; }
    inline const char* getStringKey() const { return skey_.c_str(); }
    inline int32_t getIntKey() const { return ikey_; }

    // fill a serial buffer
    int fillSB( SerialBuffer& sb, int32_t serial = 0 );
    int readSB( SerialBuffer& sb );

    PersCommand* command() { return cmd_.get(); }

    inline int status() const { return cmd_->status(); }
    void setStatus( int, const char* what = 0 );
    const std::string& exception() const { return exception_; }

    inline PersCall* next() { return next_; }
    inline void* context() { return context_; }
    inline void setContext( void* ctx ) { context_ = ctx; }
    inline PersCallInitiator* initiator() { return initiator_; }

private:
    ProfileType                   type_;
    std::auto_ptr< PersCommand >  cmd_;
    std::string                   skey_;
    int32_t                       ikey_;
    std::string                   exception_;
    PersCall*                     next_;     // managed from PersClient
    void*                         context_;  // external ownership
    PersCallInitiator*            initiator_;
};


}
}
}

#endif /* ! _SCAG_PERS_UTIL_PERSCALLPARAMS_H */
