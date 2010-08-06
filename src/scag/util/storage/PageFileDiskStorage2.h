#ifndef _SCAG_UTIL_STORAGE_PAGEFILEDISKSTORAGE2_H
#define _SCAG_UTIL_STORAGE_PAGEFILEDISKSTORAGE2_H

#include <vector>
#include "logger/Logger.h"
#include "core/buffers/PageFile.hpp"
#include "KeyLogger.h"

namespace scag2 {
namespace util {
namespace storage {

class PageFileDiskStorage2
{
public:
    typedef smsc::core::buffers::File::offset_type index_type;
    typedef smsc::core::buffers::PageFile          storage_type;
    typedef std::vector< unsigned char >           buffer_type;

    static const bool        updatable = false;

    PageFileDiskStorage2( storage_type* pf, smsc::logger::Logger* thelog = 0 ) :
    store_(pf), log_(thelog), keylogger_(&dummyKeyLogger_), invalidIndex_(0) {
        if (log_) {
            smsc_log_debug(log_,"pagefilediskstorage created");
        }
    }

    ~PageFileDiskStorage2() { delete store_; }
    
    inline void setKeyLogger( const KeyLogger& kl ) { keylogger_ = &kl; }

    inline index_type invalidIndex() const { return invalidIndex_; }

    index_type append( buffer_type& buf ) {
        if (buf.empty()) return invalidIndex_;
        const index_type i = store_->Append( &(buf[0]), buf.size() );
        if (log_) {
            smsc_log_debug(log_,"append: key=%s buf=%u -> index=%llx",
                           keylogger_->toString(), unsigned(buf.size()),
                           static_cast<unsigned long long>(i));
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
        index_type j;
        store_->Read(i,buf,&j);
        if ( i != j ) {
            if (log_) {
                smsc_log_warn(log_,"diff index: was=%llx is=%llx buf=%u",
                              static_cast<unsigned long long>(i),
                              static_cast<unsigned long long>(j),
                              unsigned(buf.size()));
            }
        } else {
            if (log_) {
                smsc_log_debug(log_,"ok: buf=%u",
                               unsigned(buf.size()));
            }
        }
        return true;
    }

    inline void remove( index_type i, buffer_type* oldbuf = 0 ) {
        const unsigned del = store_->Delete(i);
        if (log_) {
            smsc_log_debug(log_,"remove: key=%s index=%llx -> pages=%u",
                           keylogger_->toString(),
                           static_cast<unsigned long long>(i),
                           del );
        }
    }

private:
    storage_type*         store_;
    smsc::logger::Logger* log_;
    DummyKeyLogger        dummyKeyLogger_;
    const KeyLogger*      keylogger_;
    const index_type      invalidIndex_;
};

}
}
}

#endif
