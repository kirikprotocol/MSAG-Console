#include <string>
#include <list>
#include <cstdlib>  // random
#include <string.h> // memcpy
#include <memory>
#include "Storage.h"

// for __require__
#ifndef NOLOGGERPLEASE
#include <util/debug.h>
#else
#define __require__(x)
#endif


using namespace scag::util::storage;

static smsc::logger::Logger* cachelog = NULL;

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
                throw std::runtime_error( "SimpleMemoryStorage: cannot create a storage with zero size" );
            }
            array_ = new Itemlist[ cachesize_ ];
        }
        ~SimpleMemoryStorageBase() {
            // we have to delete all elements
            delete[] array_;
        }

    public:

        bool set( const key_type& k, value_type* v ) {
            smsc_log_debug( cachelog, "set: %s", k.toString().c_str() );
            return array_[hash(k)].set(k,v);
        }
        
        value_type* get( const key_type& k ) const {
            value_type* v = array_[hash(k)].get(k);
            smsc_log_debug( cachelog, "get: %s %s", k.toString().c_str(), v ? "hit" : "miss" );
            return v;
        }

        value_type* release( const key_type& k ) {
            smsc_log_debug( cachelog, "clr: %s", k.toString().c_str() );
            return array_[hash(k)].release(k);
        }

    private:
        inline uint32_t hash( const key_type& k ) const {
            return key_type::CalcHash(k) % cachesize_;
        }

        struct Itemlist // : public MemoryStorage< key_type, value_type > 
        {
            typedef std::list< std::pair< key_type, value_type* > >  SList;

            Itemlist() {}

            ~Itemlist() {
                smsc_log_debug( cachelog, "delete itemlist: size=%d", list_.size() );
                for ( typename SList::iterator i = list_.begin();
                      i != list_.end();
                      ++i ) {
                    smsc_log_debug( cachelog, "del: %s", i->first.toString().c_str() );
                    delete i->second;
                    i->second = NULL;
                }
            }


            bool set( const key_type& k, value_type* v ) {
                typename SList::iterator i = list_.begin();
                for ( ; i != list_.end(); ++i ) {
                    if ( i->first == k ) break;
                }
                if ( i != list_.end() ) {
                    // found
                    if ( i->second ) {
                        if ( v ) {
                            delete v; // to avoid memleak

                            // FIXME: should we replace with warning?
                            throw std::runtime_error
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
                for ( typename SList::const_iterator i = list_.begin();
                      i != list_.end();
                      ++i ) {
                    if ( i->first == k ) return i->second;
                }
                return NULL;
            }

            value_type* release( const key_type& k )  {
                for ( typename SList::iterator i = list_.begin();
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
        friend class SimpleMemoryStorage< Key, Val >;
        typedef SimpleMemoryStorage< Key, Val >  Base;
        typedef typename Base::Itemlist  Itemlist;
        typedef typename Itemlist::SList SList;

    public:
        typedef Key key_type;
        typedef Val value_type;

        void reset() {
            pos_ = 0;
            i_ = base_->array_[pos_].list_.begin();
        }
        bool next( key_type& k, value_type*& v ) {
            if ( pos_ < base_->cachesize_ ) {
                while ( i_ == base_->array_[pos_].list_.end() ) {
                    ++pos_;
                    if ( pos_ >= base_->cachesize_ ) return false;
                    i_ = base_->array_[pos_].list_.begin();
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
        const Base*                          base_;
        unsigned int                         pos_;
        typename SList::const_iterator       i_;
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

/**
* Файл содержит описание внутренней структуры данных для представления SMS
* в системе SMS центра. Используется системой хранения.
*
* @author Victor V. Makarov
* @version 1.0
* @see MessageStore
*/

const int MAX_ADDRESS_VALUE_LENGTH = 20;
typedef char        AddressValue[MAX_ADDRESS_VALUE_LENGTH+1];

struct Address
{
  uint8_t      length, type, plan;
  AddressValue value;

  /**
  * Default конструктор, просто инициализирует некоторые поля нулями
  */
  Address() : length(1), type(0), plan(0)
  {
    value[0] = '0'; value[1] = '\0';
  };

  /**
  * Конструктор для Address, инициализирует поля структуры реальными данными.
  * Копирует даннуе из буфера к себе
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
  * Конструктор копирования, используется для создания адреса по образцу
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
  * Переопределённый оператор '=',
  * используется для копирования адресов друг в друга
  *
  * @param addr   Правая часть оператора '='
  * @return Ссылку на себя
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
  * Метод устанавливает значение адреса и его длинну.
  * Длинна адреса должна быть меньше MAX_ADDRESS_VALUE_LENGTH.
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
  * Метод копирует значение адреса и возвращает его длинну
  *
  * @param _value указатель на буфер куда будет скопированно значение адреса
  *               буфер должен иметь размер не меньше
  *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
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
  * Возвращает длинну адреса
  *
  * @return длинна адреса
  */
  inline uint8_t getLength() const
  {
    return length;
  };

  /**
  * Устанавливает тип адреса
  *
  * @param _type  тип адреса
  */
  inline void setTypeOfNumber(uint8_t _type)
  {
    type = _type;
  };

  /**
  * Возвращает тип адреса
  *
  * @param _type  тип адреса
  */
  inline uint8_t getTypeOfNumber() const
  {
    return type;
  };

  /**
  * Устанавливает план нумерации адреса
  *
  * @param _plan  план нумерации адреса
  */
  inline void setNumberingPlan(uint8_t _plan)
  {
    plan = _plan;
  };

  /**
  * Возвращает план нумерации адреса
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
  inline const std::string toString()const
  {
    if(length>32)abort();
    char vl[33];
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

        const std::string toString() const {
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
        Session() : lastAccessTime(-1), somedata("sessionData") {}
        Session( const CSessionKey& sk ) :
        sessionKey(sk), lastAccessTime(-1), somedata("sessionData") {}

    public:
        void serialize( Serializer& pfb ) const;
        void deserialize( Deserializer& pfb );
        const CSessionKey& getKey() const { return sessionKey; }

    private:
        Session( const Session& );
        
    private:
        CSessionKey sessionKey;
        time_t      lastAccessTime;
        std::string somedata;
    };

    typedef Session SerializableSession;


    void Session::serialize( Serializer& pfb ) const
    {
        pfb << sessionKey.toString() << uint32_t(lastAccessTime) << somedata;
    }


    void Session::deserialize( Deserializer& pfb )
    {
        std::string s;
        uint32_t tm;
        pfb >> s >> tm >> somedata;
        sessionKey = CSessionKey(Address(s.c_str()));
        lastAccessTime = time_t(tm);
    }

    Serializer& operator << ( Serializer& s, const Session& ss ) {
        ss.serialize( s );
        return s;
    }

    Deserializer& operator >> ( Deserializer& s, Session& ss ) {
        ss.deserialize( s );
        return s;
    }

} // namespace


typedef SimpleMemoryStorage< CSessionKey, Session > MemStorage;
typedef PageFileDiskStorage< SerializableSession > DiskDataStorage;
typedef RBTreeIndexStorage< CSessionKey, DiskDataStorage::index_type > DiskIndexStorage;
typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
typedef CachedDiskStorage< MemStorage, DiskStorage > SessionStorage;


unsigned int checkStorage( SessionStorage*       store,
                           smsc::logger::Logger* log );


int main( int argc, char** argv )
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* log = smsc::logger::Logger::getInstance("sessions");

    cachelog = smsc::logger::Logger::getInstance("cache");

    smsc_log_info( log, "============================================================" );
    smsc_log_info( log, "===    STARTING UP TESTRB    ===============================" );
    smsc_log_info( log, "============================================================" );

    int initrand = 11;
    if ( argc > 1 ) {
        initrand = atoi( argv[1] );
    }
    srandom( initrand );
    for ( size_t i = 0; i < 10; ++i )
        smsc_log_info( log, "a random: %ld", random() );

    // configs
    const std::string storagename = "sessions";
    const std::string storagepath = ".";
    const int indexgrowth = 1000;
    // const int datablocksz = 0;
    // const int blocksinfile = 0;
    const int cachesize = 1000;
    const int pagesize = 1024;
    const int preallocate = 1024;

    unsigned int interval = 1000000;
    if ( getenv("interval") ) {
        interval = strtoul(getenv("interval"), NULL, 10 );
    }
    unsigned int totalpasses = 10;
    if ( getenv("totalpasses") ) {
        totalpasses = strtoul(getenv("totalpasses"), NULL, 10 );
    }

    const unsigned int flushprob = 3;
    const unsigned int cleanprob = 10;

    std::auto_ptr< SessionStorage > store;

    unsigned int totalflushes = 0;
    unsigned int totalflushsessions = 0;
    unsigned int totalcleans = 0;
    unsigned int totalcleansessions = 0;
    unsigned int totalmisses = 0;

    {
        std::auto_ptr< MemStorage > ms( new MemStorage( cachesize ) );
        smsc_log_debug( log, "memory storage created" );
        std::auto_ptr< PageFile > pf( new PageFile );
        std::string fn( storagepath + '/' + storagename + '/' + storagename + "-data" );
        try {
            pf->Open( fn );
        } catch (...) {
            pf->Create( fn, pagesize, preallocate );
        }
        smsc_log_debug( log, "pagefile storage created" );
        std::auto_ptr< DiskDataStorage > dds( new DiskDataStorage( pf.release() ) );
        smsc_log_debug( log, "data disk storage created" );
        std::auto_ptr< DiskIndexStorage > dis( new DiskIndexStorage( storagename,
                                                                     storagepath,
                                                                     indexgrowth ));
        smsc_log_debug( log, "data index storage created" );
        std::auto_ptr< DiskStorage > ds( new DiskStorage( dis.release(), dds.release() ) );
        smsc_log_debug( log, "disk storage assembled" );
        store.reset( new SessionStorage( ms.release(), ds.release() ) );
        smsc_log_debug( log, "session storage assembled" );
    }

    fprintf( stderr, "STARTING STORAGE CHECK\n" );
    smsc_log_debug( log, "STARTING STORAGE CHECK" );
    unsigned int precount = checkStorage( store.get(), log );
    fprintf( stderr, "STORAGE CHECK FINISHED\n" );
    smsc_log_debug( log, "STORAGE CHECK FINISHED" );

    for ( size_t i = 0; i < totalpasses; ++i ) {

        if ( i % 100 == 0 )
            smsc_log_debug( log, "pass #%d", i );

        uint8_t ton = random() ? 0 : 1;
        uint8_t npi = 1;
        char buf[20];
        int len = sprintf( buf, "8913%07d", random() % interval );

        const CSessionKey sk( Address(len,ton,npi,buf) );
        Session* v = store->get( sk );
        if ( ! v ) {
            smsc_log_debug( log, "mis: %s ... added to cache", sk.toString().c_str() );
            v = new Session( sk );
            store->set( sk, v );
            ++totalmisses;
        } else {
            smsc_log_debug( log, "hit: %s, session key: %s", sk.toString().c_str(), v->getKey().toString().c_str() );
            if ( v->getKey() != sk ) {
                
                fprintf( stderr, "WARNING: different key found %s != %s\n",
                         sk.toString().c_str(), v->getKey().toString().c_str() );
                smsc_log_error( log, "WARNING: different key found %s != %s",
                                sk.toString().c_str(), v->getKey().toString().c_str() );
                delete store->purge( sk );
                v = new Session( sk );
                store->set( sk, v );
            }
        }


        if ( random() % 100 < flushprob ) {

            // flushing
            smsc_log_debug( log, "STARTING FLUSHING CACHE" );
            ++totalflushes;
            totalflushsessions += store->flush();
            smsc_log_debug( log, "FLUSH FINISHED, STARTING CLEANING" );

            // remove all cached objects
            // 10 % chance of clean objects even from disk
            const bool fromdisk = ( random() % 100 < cleanprob );
            if ( fromdisk ) {
                ++totalcleans;
                smsc_log_debug( log, "CLEAN ALSO WILL AFFECT DISK" );
            }

            CSessionKey k;
            Session* dummy;
            for ( SessionStorage::iterator_type j( store->begin() ); j.next( k, dummy ); ) {
                smsc_log_debug( log, "delete: %s", k.toString().c_str() );
                if ( ! fromdisk ) {
                    dummy = store->purge( k );
                } else {
                    dummy = store->release( k );
                    ++totalcleansessions;
                }
                delete dummy;
            }
            smsc_log_debug( log, "CACHE CLEANED" );

        }

    }

    store->flush();

    // printout the statistics
    printf( "total accesses      : %d\n", totalpasses );
    printf( "total cache hits    : %d\n", store->hitcount() );
    printf( "total disk hits     : %d\n", totalpasses - store->hitcount() - totalmisses );
    printf( "total misses        : %d\n", totalmisses );
    printf( "total flushes       : %d\n", totalflushes );
    printf( "total flushed items : %d\n", totalflushsessions );
    printf( "total cleans        : %d\n", totalcleans );
    printf( "total cleaned items : %d\n", totalcleansessions );
    printf( "cache size          : %d\n", cachesize );
    printf( "number interval     : %d\n", interval );

    fprintf( stderr, "STARTING STORAGE CHECK\n" );
    smsc_log_debug( log, "STARTING STORAGE CHECK" );
    unsigned int postcount = checkStorage( store.get(), log );
    fprintf( stderr, "STORAGE CHECK FINISHED\n" );
    smsc_log_debug( log, "STORAGE CHECK FINISHED" );

    printf( "items on disk (pre) : %d\n", precount );
    printf( "items on disk (post): %d\n", postcount );
    return 0;
}



unsigned int checkStorage( SessionStorage* store, smsc::logger::Logger* log )
{
    CSessionKey prevk;
    CSessionKey k;
    Session s;
    unsigned int count = 0;
    for ( DiskStorage::iterator_type i = store->dataBegin();
          i.next(k,s);
          ) {
        ++count;
        if ( count > 50000 ) {
            fprintf( stderr, "store check failed\n" );
            break;
        }
        if ( ( count > 1 ) && ( k < prevk ) ) {
            fprintf( stderr, "WARNING: keys out of order: %s >= %s\n",
                     prevk.toString().c_str(),
                     k.toString().c_str() );
            smsc_log_error( log, "WARNING: keys out of order: %s >= %s\n",
                            prevk.toString().c_str(),
                            k.toString().c_str() );
        }
        prevk = k;
        // if ( count % 100 == 0 ) {
        // fprintf( stderr, "store check pass #%d, key=%s\n", count, k.toString().c_str() );
        smsc_log_debug( log, "store check pass #%d, key=%s", count, k.toString().c_str() );
        // }
        if ( k != s.getKey() ) {
            fprintf( stderr, "WARNING: key mismatch: %s != %s\n",
                     k.toString().c_str(),
                     s.getKey().toString().c_str() );
            smsc_log_error( log, "WARNING: key mismatch: %s != %s",
                            k.toString().c_str(),
                            s.getKey().toString().c_str() );
        }
    }
    return count;
}
