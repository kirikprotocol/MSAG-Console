#ifndef _SCAG_UTIL_STORAGE_BLOCKSHSBACKUPDATA_H
#define _SCAG_UTIL_STORAGE_BLOCKSHSBACKUPDATA_H

namespace scag {
namespace util {
namespace storage {

/// backup data for BlocksHSStorage.
/// NOTE: it is intentively detached from stored items.
/// NOTE: method names are as in the original classes by V.Odarchenko.
class BlocksHSBackupData
{
public:
    BlocksHSBackupData() {}
    ~BlocksHSBackupData() {}

    /// block list backup
    void addDataToBackup( long nextblock ) {
        backup_.push_back( nextblock );
    }
    void clearBackup() {
        backup_.clear();
    }
    const std::vector< long >& getBackup() const {
        return backup_;
    }
    void setBackup( const std::vector<long>& bkp ) {
        backup_ = bkp;
    }
    
    /// previous state backup
    unsigned getBackupDataSize() const {
        return previous_.length();
    }
    const char* getBackupData() const {
        return previous_.c_str();
    }
    void setBackupData( const char* data, unsigned sz ) {
        previous_.assign( data, sz );
    }
    /// NOTE: restoreBackup should not be here

private:
    std::vector< long > backup_;     // indices of datablocks
    std::string         previous_;   // previous serialized state
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_BLOCKSHSBACKUPDATA_H */
