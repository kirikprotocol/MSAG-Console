#ifndef _SCAG_UTIL_STORAGE_BHDISKSTORAGE2_H
#define _SCAG_UTIL_STORAGE_BHDISKSTORAGE2_H

// #include "BlocksHSStorage.h"
#include "DataBlockBackup2.h"
#include "BlocksHSStorage2.h"
#include "scag/util/io/HexDump.h"

namespace scag2 {
namespace util {
namespace storage {

/**
 * BlocksHSStorage wrapper
 */
template < class Key, class Val, class Storage = BlocksHSStorage2 >
class BHDiskStorage2
{
public:
    static const bool                          updatable = true;
    typedef Storage                            storage_type;
    typedef Key                                key_type;
    typedef typename storage_type::index_type  index_type;
    typedef DataBlockBackup2< Val >            value_type;
    typedef typename storage_type::buffer_type buffer_type;

    BHDiskStorage2( storage_type* hs,
                    io::GlossaryBase* glossary = 0,
                    smsc::logger::Logger* logger = 0 ) :
    store_(hs), glossary_(glossary), v_(0), buf_(0), log_(logger)
    {
        i_ = invalidIndex();
        if ( !hs ) {
            throw std::runtime_error("BlocksHSStorage2 should be provided" );
        }
    }


    ~BHDiskStorage2() {
        delete store_; 
        value_type::deallocBackup(buf_);
    }


    inline bool setKey( const key_type& k ) const 
    {
        key_ = k;
        return true;
    }


    inline index_type invalidIndex() const {
        return store_->invalidIndex();
    }


    void serialize( const value_type& v ) /* throw exception ? */
    {
        i_ = invalidIndex();
        v_ = const_cast<value_type*>(&v);
        if ( !buf_ ) buf_ = value_type::allocBackup();
        if (log_) smsc_log_debug(log_,"serialize(%s)",v.toString().c_str());
        io::Serializer ser(*buf_,glossary_);
        ser.setVersion(store_->version());
        ser.reset();
        ser.setwpos( headerSize()+extraSize() );
        ser << key_;
        const size_t oldwpos = ser.wpos();
        ser << *v.value;
        if ( ser.wpos() <= oldwpos ) {
            // no bytes written
            ser.reset();
        } else {
            packBuffer(*buf_,0);
        }
    }


    index_type append() /* throw exception */
    {
        index_type blockIndex;
        if ( v_ && buf_ && !buf_->empty() ) {
            blockIndex = store_->change(invalidIndex(),0,buf_);
            if ( blockIndex != invalidIndex() ) {
                // successfully stored, now buf_ has fully functional
                attachBackup(*v_,buf_);
            }
            if (log_) {
                smsc_log_debug(log_,"append: %s idx=%llx %s",
                               key_.toString().c_str(),
                               uint64_t(blockIndex),
                               v_->toString().c_str());
            }
        } else {
            blockIndex = invalidIndex();
        }
        return blockIndex;
    }
    

    bool read( index_type i ) const /* throw exception */
    {
        i_ = i;
        // v_ = 0;
        if (!buf_) buf_ = value_type::allocBackup();
        buf_->clear();
        return store_->read(i,*buf_);
    }


    bool deserialize( value_type& v ) const /* throw exception */
    {
        if ( !buf_ || buf_->empty() || !v.value ) return false;
        if ( ! deserializeBuffer(*v.value,*buf_) ) return false;
        attachBackup(v,buf_);
        if (log_) { smsc_log_debug(log_,"deser: %s idx=%llx %s",
                                   key_.toString().c_str(),
                                   uint64_t(i_),
                                   v.toString().c_str() ); 
        }
        return true;
    }


    index_type update( index_type i )
    {
        index_type blockIndex;
        if ( v_ && buf_ && !buf_->empty() && i != invalidIndex() ) {
            blockIndex = store_->change(i,v_->backup,buf_);
            if ( blockIndex != invalidIndex() ) {
                // successfully stored, attach buf_ to v
                attachBackup(*v_,buf_);
            }
            if (log_) {
                smsc_log_debug(log_,"update: %s idx=%llx %s",
                               key_.toString().c_str(),
                               uint64_t(blockIndex),
                               v_->toString().c_str());
            }
        } else {
            blockIndex = invalidIndex();
        }
        return blockIndex;
    }


    void remove(index_type i)
    {
        store_->change(i,0,0);
    }


    bool recoverFromBackup( value_type& v )
    {
        return (v.value && v.backup && deserializeBuffer(*v.value,*v.backup));
    }


    template < class DiskIndexStorage >
        class IndexRescuer : public storage_type::IndexRescuer
    {
    public:
        IndexRescuer( DiskIndexStorage& istore,
                      BHDiskStorage2&   dstore ) :
        istore_(istore), dstore_(dstore) {}

        int recover( const std::string& dbname,
                      const std::string& dbpath )
        {
            return dstore_.store_->recover(dbname,dbpath,this);
        }

        virtual void recoverIndex( index_type idx, buffer_type& buffer ) {
            dstore_.unpackBuffer(buffer,0);
            io::Deserializer dsr(buffer);
            dsr.setVersion(dstore_.store_->version());
            dsr.setrpos(dstore_.headerSize()+dstore_.extraSize());
            typename DiskIndexStorage::key_type key;
            dsr >> key;
            istore_.setIndex(key,idx);
        }

    private:
        DiskIndexStorage& istore_;
        BHDiskStorage2&   dstore_;
    };


protected:
    bool deserializeBuffer( typename value_type::value_type& value,
                            buffer_type& buffer ) const
    {
        if (buffer.empty()) return false;
        buffer_type headers;
        unpackBuffer(buffer,&headers);
        io::Deserializer dsr(buffer,glossary_);
        dsr.setVersion(store_->version());
        bool rv = false;
        key_type key;
        try {
            dsr.setrpos(headerSize()+extraSize());
            dsr >> key;
            // should we check for key match here?
            if ( key != key_ ) {
                key_type empty;
                if ( key_ != empty ) {
                    if (log_) smsc_log_warn(log_,"key mismatch: idx=%llx, key=%s, key_=%s",
                                            uint64_t(i_), key.toString().c_str(), key_.toString().c_str());
                    smsc::util::Exception("key mismatch: idx=%llx, key=%s, key_=%s",
                                          uint64_t(i_), key.toString().c_str(), key_.toString().c_str());
                }
            }
            dsr >> value;
            rv = true;
        } catch ( std::exception& e ) {
            if (log_) {
                smsc_log_warn( log_,"exc in BHS2: %s, idx=%llx oldkey=%s key=%s bufSz=%u",
                               e.what(),
                               uint64_t(i_),
                               key_.toString().c_str(),
                               key.toString().c_str(),
                               unsigned(buffer.size()) );
                if (log_->isDebugEnabled()) {
                    const size_t ipos = headerSize()+extraSize();
                    if ( buffer.size() > ipos ) {
                        const size_t bsz = buffer.size()-ipos;
                        HexDump hd;
                        HexDump::string_type dump;
                        dump.reserve(hd.hexdumpsize(bsz)+hd.strdumpsize(bsz)+10);
                        hd.hexdump(dump,&buffer[ipos],bsz);
                        hd.strdump(dump,&buffer[ipos],bsz);
                        smsc_log_debug(log_,"buf: %s", hd.c_str(dump));
                    }
                }
            }
        }
        // pack buffer back again
        packBuffer(buffer,&headers);
        return rv;
    }
    
    inline size_t headerSize() const {
        return store_->headerSize();
    }
    /// reserved space
    inline size_t extraSize() const { return 32; }
    inline void packBuffer(buffer_type& buf, buffer_type* hdr) const {
        store_->packBuffer(buf,hdr);
    }
    inline void unpackBuffer( buffer_type& buf, buffer_type* hdr ) const {
        store_->unpackBuffer(buf,hdr);
    }
    inline void attachBackup( value_type& val, buffer_type*& newBuf ) const {
        if (log_) {
            smsc_log_debug(log_,"val @ %p { value=%p, backup=%p } w/ key=%s, buffer %p attached",
                           &val, val.value, val.backup, key_.toString().c_str(), newBuf );
        }
        std::swap(val.backup,newBuf);
    }

private:
    storage_type*            store_; // owned
    io::GlossaryBase*        glossary_; // not owned
    mutable key_type         key_;
    mutable index_type       i_;
    mutable value_type*      v_;     // not owned
    mutable buffer_type*     buf_;   // owned
    smsc::logger::Logger*    log_;
};

} // namespace storage
} // namespace util
} // namespace scag

namespace scag {
namespace util {
namespace storage {
using namespace scag2::util::storage;
} // namespace storage
} // namespace util
} // namespace scag

#endif /* _SCAG_UTIL_STORAGE_BHDISKSTORAGE2_H */
