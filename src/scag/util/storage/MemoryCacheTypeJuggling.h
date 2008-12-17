#ifndef _SCAG_UTIL_STORAGE_MEMORYCACHETYPEJUGGLING_H
#define _SCAG_UTIL_STORAGE_MEMORYCACHETYPEJUGGLING_H

namespace scag {
namespace util {
namespace storage {

template < class Val >
class MemoryCacheSimpleTypeJuggling
{
public:
    typedef Val         value_type;
    typedef value_type* stored_type;
    // typedef value_type& ref_type;    

    inline value_type* store2val( stored_type v ) const {
        return v;
    }
    inline stored_type val2store( value_type* v ) const {
        return v;
    }
    // arg should be ref to store but since it is a ptr we use pass by value
    inline value_type& store2ref( stored_type v ) const {
        return *v;
    }
    inline void dealloc( stored_type v ) const {
        delete v;
    }

protected:
    /// release the value_type part only
    inline void releaseval( stored_type& v ) const {
        v = NULL;
    }
};

} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_MEMORYCACHETYPEJUGGLING_H */
