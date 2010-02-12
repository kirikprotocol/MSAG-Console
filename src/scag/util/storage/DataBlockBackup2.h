#ifndef _SCAG_UTIL_STORAGE_DATABLOCKBACKUP2_H
#define _SCAG_UTIL_STORAGE_DATABLOCKBACKUP2_H

// bukind, 2008-06-23 backup functionality extracted from scag/pers/Profile

#include <vector>
#include "logger/Logger.h"
#include "scag/util/io/Serializer.h"

namespace scag2 {
namespace util {
namespace storage {

struct DataBlockBackup2Base
{
private:
    static void initLog();
    static smsc::logger::Logger* log_;

public:
    typedef io::Serializer::Buf     backup_type;

    DataBlockBackup2Base( backup_type* b = 0 ) : backup(b) {}
    /*
     * default copying is ok
    DataBlockBackup2Base( const DataBlockBackup2Base& b ) : backup(b.backup) {}
    DataBlockBackup2Base& operator = ( const DataBlockBackup2Base& b ) {
        backup = b.backup;
        return *this;
    }
     */

    inline static backup_type* allocBackup() {
        initLog();
        backup_type* bck = new backup_type;
        smsc_log_debug(log_,"ctor %p", bck);
        return bck;
    }

    inline static void deallocBackup( backup_type* bck ) {
        initLog();
        smsc_log_debug(log_,"dtor %p", bck);
        delete bck;
    }

protected:
    ~DataBlockBackup2Base() {}
public:
    mutable backup_type* backup;
};


/// template backup class working in cooperation with BlocksHSStorage.
/// It simply adds backup functionality to class Val.
/// Requirements on Val: none
template < class Val > struct DataBlockBackup2 : public DataBlockBackup2Base
{
    typedef Val                 value_type;

    DataBlockBackup2(value_type* v = NULL, backup_type* b = NULL) : DataBlockBackup2Base(b), value(v) {}
    // default copying is ok
    // DataBlockBackup2( const DataBlockBackup2& b ) : DataBlockBackup2Base(b.backup), value(b.value) {}
    /*
    DataBlockBackup2& operator = ( const DataBlockBackup2& b ) {
        backup = b.backup;
        value = b.value;
        return *this;
    }
     */

    inline void dealloc() {
        if (value) { delete value; value = 0; }
        if (backup) { deallocBackup(backup); backup = 0; }
    }

    std::string toString() const {
        char buf[100];
        snprintf(buf,sizeof(buf),"v=%p b=%p %s",value,backup,value ? value->toString().c_str() : "");
        return buf;
    }


public:
    Val*                 value;
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
        if (!v.backup) v.backup = stored_type::allocBackup();
        return v;
    }

    inline void dealloc( stored_type& v ) const {
        v.dealloc();
    }

    std::string storedTypeToString( stored_type v ) const {
        return v.toString();
    }

    /*
    inline value_type* releaseval( stored_type& v ) const {
        value_type* x = v.value;
        v.value = 0;
        return x;
    }
     */

    inline value_type* setval( stored_type& v, value_type* nv ) const {
        value_type* x = v.value;
        v.value = nv;
        return x;
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
