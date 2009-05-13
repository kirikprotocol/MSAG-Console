#ifndef _SCAG_UTIL_STORAGE_DATABLOCKBACKUP2_H
#define _SCAG_UTIL_STORAGE_DATABLOCKBACKUP2_H

// bukind, 2008-06-23 backup functionality extracted from scag/pers/Profile

#include <vector>
#include "logger/Logger.h"
#include "scag/util/io/Serializer.h"

namespace scag2 {
namespace util {
namespace storage {

/// template backup class working in cooperation with BlocksHSStorage.
/// It simply adds backup functionality to class Val.
/// Requirements on Val: none
template < class Val >
struct DataBlockBackup2
{
    typedef Val                 value_type;
    typedef Serializer::Buf     backup_type;

    DataBlockBackup2(value_type* v = NULL, backup_type* b = NULL) : value(v), backup(b) {}

public:
    Val*                 value;
    mutable backup_type* backup;
};


/// a policy for type juggling for use with HashedMemoryCache.
/// Requirements on Val:
///   destructible
template < class Val >
class DataBlockBackupTypeJuggling2
{
public:
    typedef Val                               value_type;
    typedef DataBlockBackup2< Val >           stored_type;
    typedef typename stored_type::backup_type backup_type;
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
        if (!v.backup) v.backup = new backup_type;
        return v;
    }

    inline void dealloc( stored_type& v ) const {
        if (v.value) { delete v.value; v.value = 0; }
        if (v.backup) { delete v.backup; v.backup = 0; }
    }
protected:
    inline void releaseval( stored_type& v ) const {
        v.value = 0;
    }
};


} // namespace storage
} // namespace util
} // namespace scag2

/*
template < class T >
inline scag::util::storage::Serializer& operator << (scag::util::storage::Serializer& ser, 
                                                     const scag::util::storage::DataBlockBackup < T > &dataBlock) { 
    size_t start = ser.size();
    ser << *(dataBlock.value);
    const char* data = reinterpret_cast<const char*>(ser.data());
    size_t end = ser.size();
    dataBlock.backup->setBackupData(data + start, static_cast<unsigned>(end - start));
    return ser; 
};

template < class T >
inline scag::util::storage::Deserializer& operator >> (scag::util::storage::Deserializer& deser,
                                                 scag::util::storage::DataBlockBackup < T > &dataBlock) { 
    uint32_t size = 0;
    const char* buf = deser.read(size);
    dataBlock.value->deserialize(buf, size, deser.getGlossary());
    dataBlock.backup->setBackupData(buf, size);
    return deser;
};
 */

namespace scag {
namespace util {
namespace storage {
using namespace scag2::util::storage;
} // namespace storage
} // namespace util
} // namespace scag

#endif /* ! _SCAG_UTIL_STORAGE_DATABLOCKBACKUP2_H */
