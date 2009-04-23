#ifndef _SCAG_UTIL_STORAGE_BHDISKSTORAGE2_H
#define _SCAG_UTIL_STORAGE_BHDISKSTORAGE2_H

// #include "BlocksHSStorage.h"
#include "DataBlockBackup2.h"
#include "BlocksHSStorage2.h"

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
                    GlossaryBase* glossary = 0,
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
        // delete glossary_;
        delete buf_;
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
        if ( !buf_ ) buf_ = new buffer_type;
        Serializer ser(*buf_,glossary_);
        ser.setVersion(store_->version());
        ser.reset();
        ser.setwpos( headerSize()+32 );
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
                attachBackup(v_->backup,buf_);
            }
        } else {
            blockIndex = invalidIndex();
        }
        return blockIndex;
    }
    

    bool read( index_type i ) const /* throw exception */
    {
        i_ = i;
        v_ = 0;
        if (!buf_) buf_ = new buffer_type;
        buf_->clear();
        return store_->read(i,*buf_);
    }


    bool deserialize( value_type& v ) const /* throw exception */
    {
        if ( !buf_ || buf_->empty() || !v.value ) return false;
        if ( ! deserializeBuffer(v,*buf_) ) return false;
        attachBackup(v.backup,buf_);
        return true;
    }


    index_type update( index_type i )
    {
        index_type blockIndex;
        if ( v_ && buf_ && !buf_->empty() && i != invalidIndex() ) {
            blockIndex = store_->change(i,v_->backup,buf_);
            if ( blockIndex != invalidIndex() ) {
                // successfully stored, attach buf_ to v
                attachBackup(v_->backup,buf_);
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


    void recoverFromBackup( value_type& v )
    {
        // if (log_) {smsc_log_warn(log_,"recover from backup is not implemented");}
        // temporary switch backup and buffer
        if (v.value && v.backup) deserializeBuffer(v,*v.backup);
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
            Deserializer dsr(buffer);
            dsr.setVersion(dstore_.store_->version());
            dsr.setrpos(dstore_.headerSize()+32);
            typename DiskIndexStorage::key_type key;
            dsr >> key;
            istore_.setIndex(key,idx);
        }

    private:
        DiskIndexStorage& istore_;
        BHDiskStorage2&   dstore_;
    };

protected:
    bool deserializeBuffer( value_type& v, buffer_type& buffer ) const
    {
        buffer_type headers;
        unpackBuffer(buffer,&headers);
        Deserializer dsr(buffer,glossary_);
        dsr.setVersion(store_->version());
        // FIXME: should we check for key match here?
        try {
            dsr.setrpos(headerSize()+32);
            dsr >> key_;
            dsr >> *v.value;
        } catch ( std::exception& e ) {
            // FIXME: should we restore from backup here?
            if (log_) {
                smsc_log_info(log_,"exc in BHS2: %s", e.what());
            }
            throw; // or should we return false?
        }
        // if everything is ok, then pack buffer back again and attach it to v
        packBuffer(buffer,&headers);
        return true;
    }
    
    inline size_t headerSize() const {
        return store_->headerSize();
    }
    inline void packBuffer(buffer_type& buf, buffer_type* hdr) const {
        store_->packBuffer(buf,hdr);
    }
    inline void unpackBuffer( buffer_type& buf, buffer_type* hdr ) const {
        store_->unpackBuffer(buf,hdr);
    }
    inline void attachBackup( buffer_type*& oldBuf, buffer_type*& newBuf ) const {
        std::swap(oldBuf,newBuf);
    }

private:
    storage_type*            store_; // owned
    GlossaryBase*            glossary_; // not owned
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
