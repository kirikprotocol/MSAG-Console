#ifndef _SCAG_UTIL_STORAGE_BHDISKSTORAGE3_H
#define _SCAG_UTIL_STORAGE_BHDISKSTORAGE3_H

#include "logger/Logger.h"
#include "BlocksHSStorage2.h"
#include "KeyLogger.h"

namespace scag2 {
namespace util {
namespace storage {

class BHDiskStorage3
{
public:
    static const bool                           updatable = true;
    typedef  BlocksHSStorage2                   storage_type;
    typedef  storage_type::index_type           index_type;
    typedef  storage_type::buffer_type          buffer_type;

    BHDiskStorage3( storage_type* hs,
                    smsc::logger::Logger* logger = 0 ) :
    store_(hs), log_(logger), keylogger_(&dummyKeyLogger_)
    {
        if ( !hs ) {
            throw std::runtime_error("BlocksHSStorage2 should be provided" );
        }
    }

    ~BHDiskStorage3() {
        delete store_; 
    }

    inline void setKeyLogger( const KeyLogger& kl ) {
        keylogger_ = &kl;
        store_->setKeyLogger(kl);
    }


    inline index_type invalidIndex() const { return store_->invalidIndex(); }


    index_type append( buffer_type& buf ) {
        if (buf.empty()) return invalidIndex();
        const index_type i = store_->change(invalidIndex(),0,&buf);
        if (log_) {
            smsc_log_debug(log_,"append: key=%s buf=%u -> index=%llx",
                           keylogger_->toString(),
                           unsigned(buf.size()),
                           uint64_t(i));
        }
        return i;
    }
    

    bool read( index_type i, buffer_type& buf ) const {
        buf.resize(0);
        if (log_) {
            smsc_log_debug(log_,"reading key=%s index=%llx",
                           keylogger_->toString(),
                           static_cast<unsigned long long>(i));
        }
        const bool res = store_->read(i,buf);
        if (res ) {
            if (log_) {
                smsc_log_debug(log_,"ok: buf=%u", unsigned(buf.size()));
            }
        }
        return res;
    }


    index_type update( index_type i, buffer_type& buf, buffer_type* oldbuf = 0 )
    {
        if ( i == invalidIndex() || buf.empty() ) {
            return invalidIndex();
        }
        if (log_) {
            smsc_log_debug(log_,"update: key=%s index=%llx buf=%u",
                           keylogger_->toString(),
                           static_cast<unsigned long long>(i),
                           unsigned(buf.size()));
        }
        i = store_->change(i,oldbuf,&buf);
        if ( i != invalidIndex() ) {
            if (log_) {
                smsc_log_debug(log_,"ok: index=%llx",
                               static_cast<unsigned long long>(i));
            }
        }
        return i;
    }


    inline void remove( index_type i, buffer_type* oldbuf = 0 )
    {
        store_->change(i,oldbuf,0);
    }

    inline void packBuffer( buffer_type& buf, buffer_type* hdr ) {
        store_->packBuffer(buf,hdr);
    }
    inline void unpackBuffer( buffer_type& buf, buffer_type* hdr ) {
        store_->unpackBuffer(buf,hdr);
    }
    inline size_t headerSize() const { return store_->headerSize(); }

private:
    storage_type*            store_; // owned
    smsc::logger::Logger*    log_;
    const DummyKeyLogger     dummyKeyLogger_;
    const KeyLogger*         keylogger_;
};

}
}
}

#endif
