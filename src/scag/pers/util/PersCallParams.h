#ifndef _SCAG_PERS_UTIL_PERSCALLPARAMS_H
#define _SCAG_PERS_UTIL_PERSCALLPARAMS_H

#include <memory>
#include "PersCommand.h"
#include "scag/re/base/LongCallContextBase.h"

namespace scag2 {
namespace pers {
namespace util {


class PersCallParams : public lcm::LongCallParams
{
public:
    PersCallParams( ProfileType pt,
                    std::auto_ptr<PersCommand> cmd ) : type_(pt), cmd_(cmd), ikey_(0) {}

    inline PersCmd cmdType() const { return cmd_->cmdType(); }
    inline ProfileType getType() const { return type_; }

    inline void setKey( const std::string& key ) { skey_ = key; ikey_ = 0; }
    inline void setKey( int32_t key ) { ikey_ = key; skey_.clear(); }

    inline const char* getStringKey() const { return skey_.c_str(); }
    inline int32_t getIntKey() const { return ikey_; }

    // fill a serial buffer
    int fillSB( SerialBuffer& sb );
    int readSB( SerialBuffer& sb );
    inline void storeResults( re::actions::ActionContext& ctx ) { cmd_->storeResults(ctx); }

    inline int status() const { return cmd_->status(); }
    void setStatus( int, const char* what = 0 );

private:
    ProfileType                   type_;
    std::auto_ptr< PersCommand >  cmd_;
    std::string                   skey_;
    int32_t                       ikey_;
};

}
}
}

#endif /* ! _SCAG_PERS_UTIL_PERSCALLPARAMS_H */
