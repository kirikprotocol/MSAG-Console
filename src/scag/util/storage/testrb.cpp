// #include "scag/sessions/Session.h"
// #include "scag/store/SerialBuffer.h"
// #include "scag/store/FSDB.h"
// #include "scag/store/DataBlockBackup.h"

#include <string>
#include <list>
#include <string.h> // memcpy
#include "Storage.h"

// for __require__
#ifndef NOLOGGERPLEASE
#include <util/debug.h>
#else
#define __require__(x)
#endif


using namespace scag::util::storage;


#if 0
template <class Key, class Profile>
class CacheItem
{
public:
    Key key;
    Profile *pf;

    CacheItem(const Key& k, Profile *p) { key = k; pf = p; };
    ~CacheItem() {
      if (pf) {
        delete pf; 
      }
    }
};


template <class Key, class Profile>
class CachedStore
{
public:
    CachedStore(): cache(NULL) {};

    virtual ~CachedStore()
    {
        if(cache != NULL)
        {
            for(int i = 0; i < max_cache_size; i++)
                if(cache[i] != NULL)
                {
//                    _storeProfile(cache[i]->key, cache[i]->pf);
//                    delete cache[i]->pf;
                    delete cache[i];
                }
            delete[] cache;
        }
//              smsc_log_debug(log, "Shutdown cached store %s....", storeName.c_str());         
    };
        
    void init(uint32_t _max_cache_size = 1000)
    {
        max_cache_size = _max_cache_size;
        cache = new CacheItem<Key,Profile>*[_max_cache_size];
        memset(cache, 0, sizeof(CacheItem<Key,Profile>*) * _max_cache_size);
        cache_log = smsc::logger::Logger::getInstance("cachestore");
    };

    Profile* getProfile(const Key& key, bool create)
    {
        uint32_t i = key.HashCode(0) % max_cache_size;
        if(cache[i] != NULL)
        {
            if(cache[i]->key == key) {
              return cache[i]->pf;
            }

            Profile* pf = _getProfile(key, create);
            if(pf)
            {
              if (cache[i]->pf) {
                delete cache[i]->pf;
              } else {
                smsc_log_warn(cache_log, "getProfile: cache[%d] profile already deleted, key=%s ",
                               i, cache[i]->key.toString().c_str());
              }
                cache[i]->key = key;
                cache[i]->pf = pf;
                return pf;
            }
            return NULL;
        }
        else
        {
            Profile* pf = _getProfile(key, create);
            if(pf)
                cache[i] = new CacheItem<Key,Profile>(key, pf);
            return pf;
        }
            
    };

    Profile* createCachedProfile(const Key& key) {
      uint32_t i = key.HashCode(0) % max_cache_size;
      Profile* pf = _createProfile(key);
      if (!pf) {
        return NULL;
      }
      if(cache[i] != NULL) {
        if (cache[i]->pf) {
          delete cache[i]->pf;
        } else {
          smsc_log_warn(cache_log, "createCachedProfile: cache[%d] profile already deleted, key=%s ",
                         i, cache[i]->key.toString().c_str());
        }
        cache[i]->key = key;
        cache[i]->pf = pf;
      } else {
        cache[i] = new CacheItem<Key,Profile>(key, pf);
      }
      return pf;
    }

    void deleteCachedProfile(const Key& key) {
      uint32_t i = key.HashCode(0) % max_cache_size;
      if(cache[i] != NULL && cache[i]->key == key) {
        if (!cache[i]->pf) {
          smsc_log_warn(cache_log, "deleteCachedProfile: cache[%d] profile already deleted, key=%s",
                         i, cache[i]->key.toString().c_str());
        }
          delete cache[i];
          cache[i] = NULL;
      }
      _deleteProfile(key);
    }

    virtual Profile* _getProfile(const Key& key, bool create) = 0;

protected:
    virtual Profile* _createProfile(const Key& key) = 0;
    virtual void _deleteProfile(const Key& key) = 0;
    //virtual void Reset() = 0;
    //virtual bool Next(Key& key, uint8_t& profile_state) = 0;
        
protected:
    uint32_t max_cache_size;
    CacheItem<Key,Profile> **cache;
    smsc::logger::Logger* cache_log;
};



template <class Key, class Profile>
    class TreeStore : public CachedStore<Key, Profile>
{
public:
    TreeStore() { };
    ~TreeStore() { smsc_log_debug(log, "Shutdown store %s", storeName.c_str()); };

    void init(  const string& storageName, const string& storagePath,
                                int indexGrowth, int blocksInFile, int dataBlockSize, int cacheSize,  smsc::logger::Logger *_log)
    {
       CachedStore<Key,Profile>::init(cacheSize);
                
        log = smsc::logger::Logger::getInstance("treestore");;
        dblog = _log;
        
        storeName = storageName;
        if(store.Init(storageName, storagePath, indexGrowth, blocksInFile, dataBlockSize) != 0)
            throw Exception("Error init abonentstore");
//              smsc_log_info(log, "Inited: cacheSize = %d", cacheSize);
        };

    void storeProfile(const Key& key, Profile *pf)
    {
        //pf->DeleteExpired();
        //        sb.Empty();
        //pf->Serialize(sb, true);
//        delete pf;  
        store.Set(key, *pf);
        //if (store.Set(key, sb)) {
          //smsc_log_debug(log, "Set return TRUE");
        //} else {
          //smsc_log_debug(log, "Set return FALSE");
        //}
    }

    Profile* _getProfile(const Key& key, bool create)
    {
      Profile *pf = new Profile(key, dblog);
      try {
        sb.Empty();
        if(store.Get(key, *pf))
        {
            //pf->Deserialize(sb, true);
            return pf;
        }
        if(create)
            return pf;
        delete pf;
        return NULL;
      } catch (...) {
        delete pf;
        throw;
      }
    };

    void Reset() {
      store.resetStorage();
    }

    bool Next(Key& key, uint16_t& state_cnt) {
      sb.Empty();
      if (store.dataStorageNext(key, sb)) {
        state_cnt = sb.ReadInt16();
        return true;
      }
      return false;
    }

protected:
    Profile* _createProfile(const Key& key) {
      return new Profile(key, dblog);
    }

    void _deleteProfile(const Key &key) {
      //store.Remove(key);
    }

protected:
    std::string storeName;
    smsc::logger::Logger* log;
    smsc::logger::Logger *dblog;    
    FSDBT<Key,Profile> store;
    SerialBuffer sb;
};


class _FakeSession : public Session, public Serializable
{

public:
    _FakeSession( const CSessionKey& key, Logger* ) :
    Session( key )
    {
    }

    void Serialize( SerialBuffer& buf, bool toFSDB = false ) const;
    void Deserialize( SerialBuffer& buf, bool fromFSDB = false );

    /// clean up the state
    void Empty();

    std::string getKey() const;
};


typedef DataBlockBackup< CSessionKey, _FakeSession >  MySession;

typedef TreeStore< CSessionKey, MySession > SessionStore;

void _FakeSession::Serialize( SerialBuffer& buf, bool ) const
{
    SessionBuffer sb;
    const_cast< _FakeSession* >(this)->Session::Serialize( sb );
    buf.blkcpy( sb.get(), sb.GetPos() );
}

void _FakeSession::Deserialize( SerialBuffer& buf, bool )
{
    SessionBuffer sb( const_cast<char*>(buf.c_ptr()), buf.length() );
    this->Session::Deserialize( sb );
}

void _FakeSession::Empty()
{
    ClearOperations();
}

std::string _FakeSession::getKey() const
{
    return getSessionKey().toString();
}

#endif


namespace {


    template < class Key, class Val > class SimpleMemoryStorageIterator;

    template < class Key, class Val >
        class SimpleMemoryStorageBase // : MemoryStorage< Key, Val >
    {
    public:
        friend class SimpleMemoryStorageIterator< Key, Val >;

        // typedef MemoryStorage< Key, Val >  Base;
        // typedef typename Base::key_type             key_type;
        // typedef typename Base::value_type           value_type;
        typedef Key key_type;
        typedef Val value_type;

    protected:
        SimpleMemoryStorageBase( unsigned int cachesize = 10000 ) :
        cachesize_( cachesize ),
        array_(0) {
            if ( cachesize_ == 0 ) {
                throw std::runtime_exception( "SimpleMemoryStorage: cannot create a storage with zero size" );
            }
            array_ = new Itemlist[ cachesize_ ];
        }
        ~SimpleMemoryStorageBase() {
            // we have to delete all elements
            delete[] array_;
        }

    public:

        bool set( const key_type& k, value_type* v ) {
            return array_[hash(k)].set(k,v);
        }
        
        value_type* get( const key_type& k ) const {
            return array_[hash(k)].get(k);
        }

        value_type* release( const key_type& k ) {
            return array_[hash(k)].release(k);
        }

    private:
        inline uint32_t hash( const key_type& k ) const {
            return k.CalcHash() % cachesize_;
        }

        struct Itemlist // : public MemoryStorage< key_type, value_type > 
        {
            typedef std::list< std::pair< key_type, value_type* > >  SList;

            Itemlist() {}

            ~Itemlist() {
                for ( SList::iterator i = SList::begin();
                      i != SList::end();
                      ++i ) {
                    delete i->second;
                }
            }


            bool set( const key_type& k, value_type* v ) {
                SList::iterator i = list_.begin();
                for ( ; i != list_.end(); ++i ) {
                    if ( i->first == k ) break;
                }
                if ( i != list_.end() ) {
                    // found
                    if ( i->second ) {
                        if ( v ) {
                            // FIXME: replace with warning
                            throw std::runtime_exception
                                ( "Itemlist: two items with the same keys found\n"
                                  "It may mean that you issue set() w/o prior get()" );
                        }
                        delete i->second;
                    }
                    i->second = v;
                    return true;
                }
                list_.push_back( std::make_pair(k,v) );
                return false;
            }

            value_type* get( const key_type& k ) const {
                for ( SList::const_iterator i = list_.begin();
                      i != list_.end();
                      ++i ) {
                    if ( i->first == k ) return i->second;
                }
                return NULL;
            }

            value_type* release( const key_type& k )  {
                for ( SList::const_iterator i = list_.begin();
                      i != list_.end();
                      ++i ) {
                    if ( i->first == k ) {
                        value_type* v = i->second;
                        list_.erase( i );
                        return v;
                    }
                }
                return NULL;
            }


        private:
            SList  list_;
        };


    private:
        unsigned int  cachesize_;
        Itemlist*     array_;
    };


    template < class Key, class Val > class SimpleMemoryStorage;

    template < class Key, class Val >
    class SimpleMemoryStorageIterator
    {
    private:
        friend class SimpleMemoryStorage< class Key, class Val >;
        typedef SimpleMemoryStorage< class Key, class Val > Base;
        typedef typename Base::Itemlist  Itemlist;
        typedef typename Itemlist::SList SList;

    public:
        typedef Key key_type;
        typedef Val value_type;

        void reset() {
            pos_ = 0;
            i_ = base_.array_[pos_].begin();
        }
        bool next( key_type& k, value_type*& v ) {
            if ( pos_ < base_.cachesize_ ) {
                while ( i_ == base_.array_[pos_].end() ) {
                    ++pos_;
                    if ( pos_ >= base_.cachesize_ ) return false;
                    i_ = base_.array_[pos_].begin();
                }
                k = i_->first;
                v = i_->second;
                // move to the next element
                ++i_;
                return true;
            }
            return false;
        }

    private:
        SimpleMemoryStorageIterator();
        // SimpleMemoryStorageIterator( const SimpleMemoryStorageIterator< Key, Val >& );
        SimpleMemoryStorageIterator
            ( const Base& base ) :
        base_(&base) {
            reset(); 
        }
        
    private:
        Base*                                base_;
        unsigned int                         pos_;
        typename SList::iterator             i_;
    };

    
    template < class Key, class Val >
        class SimpleMemoryStorage : public SimpleMemoryStorageBase< Key, Val >
    {
    public:
        typedef Key key_type;
        typedef Val value_type;
        typedef SimpleMemoryStorageIterator<Key,Val> iterator_type;

        SimpleMemoryStorage( unsigned int cachesize ) :
        SimpleMemoryStorageBase< Key, Val >( cachesize ) {}

        iterator_type begin() const {
            return iterator_type(*this);
        }
    };

} // namespace


// test classes
namespace {

const int MAX_ADDRESS_VALUE_LENGTH = 20;
typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];

struct Address
{
  uint8_t      length, type, plan;
  AddressValue value;

  /**
  * Default конструктор, просто инициализирует некоторые пол€ нул€ми
  */
  Address() : length(1), type(0), plan(0)
  {
    value[0] = '0'; value[1] = '\0';
  };

  /**
  *  онструктор дл€ Address, инициализирует пол€ структуры реальными данными.
  *  опирует даннуе из буфера к себе
  *
  * @param _len   длинна буфера _value
  * @param _type  тип адреса
  * @param _plan  план нумерации
  * @param _value значение адреса
  */
  Address(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
    : length(_len), type(_type), plan(_plan)
  {
    setValue(_len, _value);
  };

  /**
  *  онструктор копировани€, используетс€ дл€ создани€ адреса по образцу
  *
  * @param addr   образец адреса.
  */
  Address(const Address& addr)
    : length(addr.length), type(addr.type), plan(addr.plan)
  {
    setValue(addr.length, addr.value);
  };

  Address(const char* text)
  {
      if(!text || !*text) throw std::runtime_error("bad address NULL");
    AddressValue addr_value;
    int iplan,itype;
    memset(addr_value,0,sizeof(addr_value));
    int scaned = sscanf(text,".%d.%d.%20s",
      &itype,
      &iplan,
      addr_value);
    if ( scaned != 3 )
    {
      scaned = sscanf(text,"+%20[0123456789?]s",addr_value);
      if ( scaned )
      {
        iplan = 1;//ISDN
        itype = 1;//INTERNATIONAL
      }
      else
      {
        scaned = sscanf(text,"%20[0123456789?]s",addr_value);
        if ( !scaned )
              throw std::runtime_error(std::string("bad address ")+text);
        else
        {
          iplan = 1;//ISDN
          itype = 0;//UNKNOWN
        }
      }
    }
    plan = (uint8_t)iplan;
    type = (uint8_t)itype;
    length = (uint8_t)strlen(addr_value);
    if(length==0)
    {
        throw std::runtime_error(std::string("bad address ")+text);
    }
    memcpy(value,addr_value,sizeof(addr_value));
  }

  /**
  * ѕереопределЄнный оператор '=',
  * используетс€ дл€ копировани€ адресов друг в друга
  *
  * @param addr   ѕрава€ часть оператора '='
  * @return —сылку на себ€
  */
  Address& operator =(const Address& addr)
  {
    type = addr.type; plan = addr.plan;
    setValue(addr.length, addr.value);
    return (*this);
  };

  inline int operator ==(const Address& addr)const
  {
    return (length == addr.length &&
            plan == addr.plan && type == addr.type &&
            memcmp(value, addr.value, length) == 0);
  };

  inline int operator !=(const Address& addr)const
  {
    return !(*this==addr);
  };

  bool operator<(const Address& addr)const
  {
    return type<addr.type ||
           (type==addr.type && plan<addr.plan) ||
           (type==addr.type && plan==addr.plan && strcmp(value,addr.value)<0);
  }


  /**
  * ћетод устанавливает значение адреса и его длинну.
  * ƒлинна адреса должна быть меньше MAX_ADDRESS_VALUE_LENGTH.
  *
  * @param _len   длинна нового адреса
  * @param _value значение нового адреса
  */
  inline void setValue(uint8_t _len, const char* _value)
  {
    __require__(_len && _value && _value[0]
      && _len<sizeof(AddressValue));

    memcpy(value, _value, _len*sizeof(uint8_t));
    value[length = _len] = '\0';
  };

  /**
  * ћетод копирует значение адреса и возвращает его длинну
  *
  * @param _value указатель на буфер куда будет скопированно значение адреса
  *               буфер должен иметь размер не меньше
  *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы прин€ть любое значение
  * @return длинна адреса
  */
  inline uint8_t getValue(char* _value) const
  {
    __require__(_value);

    if (length)
    {
      memcpy(_value, value, length*sizeof(uint8_t));
      _value[length] = '\0';
    }
    return length;
  }

  /**
  * ¬озвращает длинну адреса
  *
  * @return длинна адреса
  */
  inline uint8_t getLength() const
  {
    return length;
  };

  /**
  * ”станавливает тип адреса
  *
  * @param _type  тип адреса
  */
  inline void setTypeOfNumber(uint8_t _type)
  {
    type = _type;
  };

  /**
  * ¬озвращает тип адреса
  *
  * @param _type  тип адреса
  */
  inline uint8_t getTypeOfNumber() const
  {
    return type;
  };

  /**
  * ”станавливает план нумерации адреса
  *
  * @param _plan  план нумерации адреса
  */
  inline void setNumberingPlan(uint8_t _plan)
  {
    plan = _plan;
  };

  /**
  * ¬озвращает план нумерации адреса
  *
  * @return план нумерации адреса
  */
  inline uint8_t getNumberingPlan() const
  {
    return plan;
  };

  inline int getText(char* buf,size_t buflen)const
  {
    char vl[32];
    memcpy(vl,value,length);
    vl[length]=0;
    if ( type == 1 && plan == 1 ){
      return snprintf(buf,buflen,"+%s",vl);
    }else if (type == 0 && plan == 1){
      return snprintf(buf,buflen,"%s",vl);
    }else
      return snprintf(buf,buflen,".%d.%d.%s",type,plan,vl);
  }
  inline int toString(char* buf,size_t buflen)const{
    char vl[32];
    memcpy(vl,value,length);
    vl[length]=0;
    return snprintf(buf,buflen,".%d.%d.%s",type,plan,vl);
  }
  inline std::string toString()const
  {
    if(length>32)abort();
    char vl[32];
    char buf[48];
    memcpy(vl,value,length);
    vl[length]=0;
    snprintf(buf,sizeof(buf),".%d.%d.%s",type,plan,vl);
    return buf;
  }
  void Clear()
  {
    value[0]=0;
    length=0;
    type=0;
    plan=0;
  }

};


    struct CSessionKey {

        CSessionKey() {}
        CSessionKey( const Address& a ) : abonentAddr(a) {}

        inline bool operator ==(const CSessionKey& sk) const
        {
            return ( this->abonentAddr == sk.abonentAddr );
        }

        inline bool operator != ( const CSessionKey& sk ) const
        {
            return ! operator==(sk);
        }

        bool operator < ( const CSessionKey& sk ) const
        {
            if ( abonentAddr < sk.abonentAddr ) return true;
            if ( sk.abonentAddr < abonentAddr ) return false;
            // if ( USR < sk.USR ) return true;
            return false;
        }

        //CSessionKey() : USR(-1) {}
        static uint32_t CalcHash(const CSessionKey& key)
        {
            uint32_t retval = key.abonentAddr.type ^ key.abonentAddr.plan;
            int i;
            for(i=0;i<key.abonentAddr.length;i++)
            {
                retval=retval*10+(key.abonentAddr.value[i]-'0');
            }
            return retval;
        }

        std::string toString() const {
            // char buf[30];
            // snprintf(buf,sizeof(buf),":%d",USR);
            return abonentAddr.toString();
        }

    public:
        Address abonentAddr;
    };




    class Session 
    {

    public:
        Session( const CSessionKey& sk ) : sessionKey(sk), lastAccessTime(-1), somedata("data") {}

    public:
        void serialize( PageFileBuffer& pfb ) const;
        void deserialize( PageFileBuffer& pfb );

    private:
        Session( const Session& );
        
    private:
        CSessionKey sessionKey;
        time_t      lastAccessTime;
        std::string somedata;
    };

    typedef Session SerializableSession;


} // namespace



int main()
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* log = smsc::logger::Logger::getInstance("sessions");

    smsc_log_info( log, "Starting up testrb" );

    // configs
    const std::string storagename = "sessions";
    const std::string storagepath = ".";
    const int indexgrowth = 0;
    // const int datablocksz = 0;
    // const int blocksinfile = 0;
    const int cachesize = 10000;
    const int pagesize = 1024;
    const int preallocate = 1024;

    typedef SimpleMemoryStorage< CSessionKey, Session > MemStorage;
    typedef PageFileDiskStorage< SerializableSession > DiskDataStorage;
    typedef RBTreeIndexStorage< CSessionKey, DiskDataStorage::index_type > DiskIndexStorage;
    typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
    typedef CachedDiskStorage< MemStorage, DiskStorage > SessionStorage;
    std::auto_ptr< SessionStorage > store;
    {
        std::auto_ptr< MemStorage > ms( new MemStorage( cachesize ) );
        std::auto_ptr< PageFile > pf( new PageFile );
        std::string fn( storagepath + '/' + storagename + '/' + storagename + "-data" );
        try {
            pf->Open( fn );
        } catch (...) {
            pf->Create( fn, pagesize, preallocate );
        }
        std::auto_ptr< DiskDataStorage > dds( new DiskDataStorage( pf.release() ) );
        std::auto_ptr< DiskIndexStorage > dis( new DiskIndexStorage( storagename,
                                                                     storagepath,
                                                                     indexgrowth ));
        std::auto_ptr< DiskStorage > ds( new DiskStorage( dis.release(), dds.release() ) );
        store.reset( new SessionStorage( ms.release(), ds.release() ) );
    }


    for ( size_t i = 0; i < 100000; ++i ) {

        uint8_t ton = random() ? 0 : 1;
        uint8_t npi = 1;
        char buf[20];
        int len = sprintf( buf, "8913765%04d", random() % 10000 );

        const CSessionKey sk( Address(len,ton,npi,buf) );
        Session* v = store->get( sk );
        if ( ! v ) {
            smsc_log_info( log, "miss: %s", sk.toString().c_str() );
            store->set( sk, new Session( sk ) );
        } else {
            smsc_log_info( log, "hit: %s", sk.toString().c_str() );
        }


        if ( random() % 100 < 5 ) {
            // 5% flushing probability
            store->flush();
            // remove all cached objects
            CSessionKey k;
            Session* dummy;
            for ( SessionStorage::iterator_type j( store->begin() ); j.next( k, dummy ); ) {
                smsc_log_info( log, "delete: %s", sk.toString().c_str() );
                store->set( k, NULL );
            }
        }

    }

    return 0;
}
