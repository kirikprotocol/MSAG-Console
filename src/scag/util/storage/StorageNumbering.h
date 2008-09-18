#ifndef _SCAG_UTIL_STORAGE_STORAGENUMBERING_H
#define _SCAG_UTIL_STORAGE_STORAGENUMBERING_H

namespace scag {
namespace util {
namespace storage {

class StorageNumbering
{
public:
    /// @return the total number of elementary storages
    /// NOTE: this number should not be changed!
    inline unsigned storages() const {
        return storages_;
    }

    static const StorageNumbering& instance();

    /// only once (?) or please use mutex (see .cpp)
    static void setInstance( unsigned nodes, unsigned storages = 100 );

    inline unsigned nodes() const { return nodes_; }

    void setNodes( unsigned nodes );
    
    /// return the number of elementary storage by the entity number
    inline unsigned storage( unsigned long long entity ) const {
        return unsigned(entity % storages());
    }

    /// return the number of responsible node by the storage number
    inline unsigned node( unsigned storage ) const {
        return storage % nodes();
    }


    inline unsigned nodeByNumber( unsigned long long entity ) const {
        return node( storage( entity ));
    }

private:
    StorageNumbering( unsigned nodes, unsigned stores ) : nodes_(nodes), storages_(stores) {}

private:
    unsigned nodes_;
    unsigned storages_;
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* !_SCAG_UTIL_STORAGE_STORAGENUMBERING_H */
