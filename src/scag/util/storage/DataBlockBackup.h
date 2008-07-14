#ifndef _SCAG_UTIL_STORAGE_DATABLOCKBACKUP_H
#define _SCAG_UTIL_STORAGE_DATABLOCKBACKUP_H

// bukind, 2008-06-23 backup functionality extracted from scag/pers/Profile

#include <vector>
#include "logger/Logger.h"
#include "BlocksHSBackupData.h"


namespace scag {
namespace util {
namespace storage {

/// template backup class working in cooperation with BlocksHSStorage.
/// It simply adds backup functionality to class Val.
/// Requirements on Val: none
template < class Val >
struct DataBlockBackup
{
    typedef Val                 value_type;
    typedef BlocksHSBackupData  backup_type;

    DataBlockBackup(value_type* v = NULL, backup_type* b = NULL) : value(v), backup(b) {}

public:
    Val*                        value;
    mutable BlocksHSBackupData* backup;
};


/// a policy for type juggling for use with HashedMemoryCache.
/// Requirements on Val:
///   destructible
template < class Val >
class DataBlockBackupTypeJuggling
{
public:
    typedef Val                      value_type;
    typedef DataBlockBackup< Val >   stored_type;
    // typedef stored_type&             ref_type;

    inline value_type* store2val( stored_type v ) const {
        return v.value;
    }

    inline stored_type val2store( value_type* v ) const {
        return stored_type(v);
    }

    inline const stored_type& store2ref( const stored_type& v ) const {
        return store2ref( const_cast<stored_type&>(v) );
    }

    inline stored_type& store2ref( stored_type& v ) const {
        // make sure the backup is here
        if (!v.backup) v.backup = new BlocksHSBackupData;
        return v;
    }

    inline void dealloc( stored_type v ) const {
        delete v.value;
        delete v.backup;
    }
protected:
    inline void releaseval( stored_type& v ) const {
        v.value = NULL;
    }
};


} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_DATABLOCKBACKUP_H */

