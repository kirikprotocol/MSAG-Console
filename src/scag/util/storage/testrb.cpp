#include <string>
#include <list>
#include <cstdlib>  // random
#include <string.h> // memcpy
#include <memory>
#include "Storage.h"
#include "core/threads/Thread.hpp"

// for __require__
#ifndef NOLOGGERPLEASE
#include <util/debug.h>
#else
#define __require__(x)
#endif


using namespace scag::util::storage;

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
        array_(0),
        cachelog_(NULL) {
            if ( cachesize_ == 0 ) {
                throw std::runtime_error( "SimpleMemoryStorage: cannot create a storage with zero size" );
            }
            array_ = new Itemlist[ cachesize_ ];
            cachelog_ = smsc::logger::Logger::getInstance("cache");
        }
        ~SimpleMemoryStorageBase() {
            // we have to delete all elements
            delete[] array_;
        }

    public:

        bool set( const key_type& k, value_type* v ) {
            smsc_log_debug( cachelog_, "set: %s", k.toString().c_str() );
            return array_[hash(k)].set(k,v);
        }
        
        value_type* get( const key_type& k ) const {
            value_type* v = array_[hash(k)].get(k);
            smsc_log_debug( cachelog_, "get: %s %s", k.toString().c_str(), v ? "hit" : "miss" );
            return v;
        }

        value_type* release( const key_type& k ) {
            smsc_log_debug( cachelog_, "clr: %s", k.toString().c_str() );
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
                // smsc_log_debug( cachelog_, "delete itemlist: size=%d", list_.size() );
                for ( typename SList::iterator i = list_.begin();
                      i != list_.end();
                      ++i ) {
                    // smsc_log_debug( cachelog_, "del: %s", i->first.toString().c_str() );
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
        unsigned int          cachesize_;
        Itemlist*             array_;
        smsc::logger::Logger* cachelog_;
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

        long long toIndex() const {
            // calculation of the index
            const std::string s = toString();
            long long idx = 0;
            for ( size_t i = 0; i < s.size(); ++i ) {
                if ( s[i] >= '0' && s[i] <= '9' ) {
                    idx = idx*10 + (s[i] - '0');
                }
            }
            return idx;
        }

    public:
        Address abonentAddr;
    };




    class Session 
    {

    public:
        Session() : lastAccessTime(-1) {
            init();
        }
        Session( const CSessionKey& sk ) :
        sessionKey(sk), lastAccessTime(-1) {
            init();
        }

    public:
        void serialize( Serializer& pfb ) const;
        void deserialize( Deserializer& pfb );
        const CSessionKey& getKey() const { return sessionKey; }

    private:
        Session( const Session& );
        void init();
        
    private:
        CSessionKey sessionKey;
        time_t      lastAccessTime;
        std::string somedata;
    };


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

    void Session::init()
    {
        const std::string s = "0123456789abcdefghijklmnopqrstuvwxyz";
        // const std::string ss = s + s + s + s + s + s + s + s;
        // somedata = "sessionData[" + ss + ss + ss + ss + ss + "]";
        somedata = "sessionData[" + s + "]";
    }


    Serializer& operator << ( Serializer& s, const Session& ss ) {
        ss.serialize( s );
        return s;
    }

    Deserializer& operator >> ( Deserializer& s, Session& ss ) {
        ss.deserialize( s );
        return s;
    }



typedef SimpleMemoryStorage< CSessionKey, Session > MemStorage;
typedef PageFileDiskStorage< Session > DiskDataStorage;
typedef RBTreeIndexStorage< CSessionKey, DiskDataStorage::index_type > DiskIndexStorage;
typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
typedef CachedDiskStorage< MemStorage, DiskStorage > SessionStorage;


struct Config {

    Config( smsc::logger::Logger* slogg ) {
        slog = slogg;
        initrand = 0;
        if ( getenv("initrand") ) {
            initrand = strtol(getenv("initrand"), NULL, 10 );
        }
        srandom( initrand );
        for ( size_t i = 0; i < 3; ++i )
            smsc_log_info( slog, "a random: %ld", random() );

        storagename = "sessions";
        storagepath = ".";
        indexgrowth = 1000;
        if ( getenv("indexgrowth") ) {
            indexgrowth = strtoul(getenv("indexgrowth"), NULL, 10 );
        }
        cachesize = 1000;
        if ( getenv("cachesize") ) {
            cachesize = strtoul(getenv("cachesize"), NULL, 10 );
        }
        pagesize = 1024;
        if ( getenv("pagesize") ) {
            pagesize = strtoul(getenv("pagesize"), NULL, 10 );
        }
        preallocate = 100;
        if ( getenv("preallocate") ) {
            preallocate = strtoul(getenv("preallocate"), NULL, 10 );
        }
        interval = 1000000;
        if ( getenv("interval") ) {
            interval = strtoul(getenv("interval"), NULL, 10 );
        }
        totalpasses = 10;
        if ( getenv("totalpasses") ) {
            totalpasses = strtoul(getenv("totalpasses"), NULL, 10 );
        }
        flushprob = 3;
        if ( getenv("flushprob") ) {
            flushprob = strtoul(getenv("flushprob"), NULL, 10 );
        }
        cleanprob = 10;
        if ( getenv("cleanprob") ) {
            cleanprob = strtoul(getenv("cleanprob"), NULL, 10 );
        }
        minkilltime = 2;
        if ( getenv("minkilltime") ) {
            minkilltime = strtoul(getenv("minkilltime"), NULL, 10 );
        }
        maxkilltime = 5;
        if ( getenv("maxkilltime") ) {
            maxkilltime = strtoul(getenv("maxkilltime"), NULL, 10 );
        }
        if ( maxkilltime <= minkilltime ) {
            maxkilltime = minkilltime + 1;
        }

        totalflushes = 0;
        totalflushsessions = 0;
        totalcleans = 0;
        totalcleansessions = 0;
        totalmisses = 0;

    };

public:
    int         initrand;
    std::string storagename;
    std::string storagepath;
    unsigned    indexgrowth;
    unsigned    cachesize;
    unsigned    pagesize;
    unsigned    preallocate;
    unsigned    interval;
    unsigned    totalpasses;
    unsigned    flushprob;
    unsigned    cleanprob;
    unsigned    minkilltime;
    unsigned    maxkilltime;

    mutable     smsc::logger::Logger* slog;

    // statistics
    mutable unsigned totalflushes;
    mutable unsigned totalflushsessions;
    mutable unsigned totalcleans;
    mutable unsigned totalcleansessions;
    mutable unsigned totalmisses;
};


class SelfKiller : public smsc::core::threads::Thread
{
public:
    SelfKiller( int mainid, int sectowait ) :
    Thread(), mainid_(mainid), wait_(sectowait) {}
    virtual ~SelfKiller() {
        // main has finished early
        printf( "main has finished early\n" );
        Kill(16);
    }
    virtual int Execute();
private:
    int mainid_;
    int wait_;
};


int SelfKiller::Execute()
{
    struct timespec req;
    req.tv_sec = wait_;
    req.tv_nsec = 0;
    nanosleep( &req, NULL );
    fprintf( stderr, "awoken from nanosleep\n" );
    pthread_kill( mainid_, SIGKILL );
}


unsigned int checkStorage( const Config&         cfg,
                           SessionStorage*       store,
                           bool&                 ok );

int testDiskIndexStorage( const Config& cfg, DiskIndexStorage* dis );
int testSessionStorage( const Config& cfg, SessionStorage* store );



int main( int argc, char** argv )
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* slog = smsc::logger::Logger::getInstance("main");

    smsc_log_info( slog, "================================" );
    smsc_log_info( slog, "===    STARTING UP TESTRB    ===" );
    smsc_log_info( slog, "================================" );

    Config cfg( slog );

    std::auto_ptr< SessionStorage > store;

    do {

        std::auto_ptr< PageFile > pf;
        std::auto_ptr< DiskDataStorage > dds;
        std::auto_ptr< DiskIndexStorage > dis;
        std::auto_ptr< DiskStorage > ds;
        std::auto_ptr< MemStorage > ms;

        // --- setup is here
        dis.reset( new DiskIndexStorage( cfg.storagename,
                                         cfg.storagepath,
                                         cfg.indexgrowth ));
        smsc_log_debug( slog, "data index storage created" );
        return testDiskIndexStorage( cfg, dis.get() );



        pf.reset( new PageFile );
        std::string fn( cfg.storagepath + '/' + cfg.storagename + '/' + cfg.storagename + "-data" );
        try {
            pf->Open( fn );
        } catch (...) {
            pf->Create( fn, cfg.pagesize, cfg.preallocate );
        }
        smsc_log_debug( slog, "pagefile storage created" );


        dds.reset( new DiskDataStorage( pf.release() ) );
        smsc_log_debug( slog, "data disk storage created" );
        

        ds.reset( new DiskStorage( dis.release(), dds.release() ) );
        smsc_log_debug( slog, "disk storage assembled" );

        ms.reset( new MemStorage( cfg.cachesize ) );
        smsc_log_debug( slog, "memory storage created" );

        store.reset( new SessionStorage( ms.release(), ds.release() ) );
        smsc_log_debug( slog, "session storage assembled" );

    } while ( false );

    if ( ! store.get() ) return 0;

    return testSessionStorage( cfg, store.get() );
}



CSessionKey genKey( const Config& cfg )
{
    uint8_t ton = random() ? 0 : 1;
    uint8_t npi = 1;
    char buf[20];
    int len = sprintf( buf, "8913%07d", random() % cfg.interval );
    return CSessionKey( Address(len,ton,npi,buf) );
}



int testDiskIndexStorage( const Config& cfg, DiskIndexStorage* dis )
{
    // необходимо проверить, что хранилище не испорчено.

    smsc::logger::Logger* slog = cfg.slog;
    fprintf( stderr, "STARTING DISK INDEX STORAGE CHECK\n" );
    smsc_log_debug( slog, "STARTING DISK INDEX STORAGE CHECK" );
    bool ok = true;

    unsigned count = 0;
    DiskIndexStorage::key_type prevk;
    DiskIndexStorage::key_type k;
    DiskIndexStorage::index_type idx;
    
    unsigned filledcount = 0;
    for ( DiskIndexStorage::iterator_type i( dis->begin() ); i.next(k,idx); ) {

        ++count;
        if ( count % 100 == 0 ) {
            smsc_log_debug( slog, "pass #%d, key=%s, idx=%lld",
                            count, k.toString().c_str(),
                            static_cast<long long>(idx) );
        }

        if ( count > 1 ) {

            // check order
            if ( k < prevk ) {
                smsc_log_warn( slog, "WARNING: key order is broken: prev=%s k=%s",
                               prevk.toString().c_str(),
                               k.toString().c_str() );
                fprintf( stderr, "WARNING: key order is broken: prev=%s k=%s",
                         prevk.toString().c_str(),
                         k.toString().c_str() );
                ok = false;
                break;
            }

        }
        prevk = k;

        // index is cleared
        if ( idx == 0 ) continue;

        // check index
        DiskIndexStorage::index_type j = k.toIndex();
        ++filledcount;
        if ( idx != j ) {
            smsc_log_warn( slog, "WARNING: key-index mismatch: key=%s index=%lld",
                           k.toString().c_str(),
                           static_cast< long long >( idx ) );
            ok = false;
        }

    } // for next
    fprintf( stderr, "DISK INDEX STORAGE CHECK FINISHED\n" );
    smsc_log_debug( slog, "DISK INDEX STORAGE CHECK FINISHED" );
    
    smsc_log_info( slog, "total indices : %d", count );
    smsc_log_info( slog, "filled indices: %d", filledcount );
    smsc_log_info( slog, "empty indices : %d", count - filledcount );
    fprintf( stderr, "total indices : %d\n", count );
    fprintf( stderr, "filled indices: %d\n", filledcount );
    fprintf( stderr, "empty indices : %d\n", count - filledcount );

    if ( ! ok ) return 1;

    // start self killer
    SelfKiller selfkill( pthread_self(),
                         random() % (cfg.maxkilltime-cfg.minkilltime) + cfg.minkilltime );
    selfkill.Start();

    // adding, and removing indices
    unsigned totalinserts = 0;
    unsigned totalremoves = 0;
    unsigned totalrestores = 0;
    for ( size_t i = 1; i <= cfg.totalpasses; ++i ) {

        const CSessionKey sk( genKey( cfg ) );
        DiskIndexStorage::index_type idx = dis->getIndex( sk );
        if ( idx ) {
            // index found
            if ( random() % 100 < cfg.cleanprob ) {
                ++totalremoves;
                dis->removeIndex( sk );
            }
        } else {
            // not found
            // fprintf( stderr, "adding key=%s idx=%lld\n", sk.toString().c_str(),
            // static_cast<long long>(sk.toIndex()) );
            // smsc_log_debug( slog, "adding key=%s idx=%lld", sk.toString().c_str(),
            // static_cast<long long>(sk.toIndex()) );
            if ( dis->setIndex( sk, sk.toIndex() ) ) {
                ++totalinserts;
            } else {
                ++totalrestores;
            }
        }

        if ( i % 100 == 0 ) {
            smsc_log_debug( slog, "pass #%d, inserts=%d, restores=%d, removes=%d, size=%d",
                            i, totalinserts, totalrestores, totalremoves, count + totalinserts );
            fprintf( stderr, "pass #%d, inserts=%d, restores=%d, removes=%d, size=%d\n",
                     i, totalinserts, totalrestores, totalremoves, count + totalinserts );
        }

    }
    return 0;
}


int testSessionStorage( const Config& cfg, SessionStorage* store )
{
    smsc::logger::Logger* slog = cfg.slog;

    fprintf( stderr, "STARTING STORAGE CHECK\n" );
    smsc_log_debug( slog, "STARTING STORAGE CHECK" );
    bool ok = true;
    unsigned int precount = checkStorage( cfg, store, ok );
    fprintf( stderr, "STORAGE CHECK FINISHED\n" );
    smsc_log_debug( slog, "STORAGE CHECK FINISHED" );
    if ( ! ok ) return 1;

    for ( size_t i = 0; i < cfg.totalpasses; ++i ) {

        if ( i % 100 == 0 )
            smsc_log_debug( slog, "pass #%d", i );

        const CSessionKey sk( genKey( cfg ) );
        Session* v = store->get( sk );
        if ( ! v ) {
            smsc_log_debug( slog, "mis: %s ... added to cache", sk.toString().c_str() );
            v = new Session( sk );
            store->set( sk, v );
            ++cfg.totalmisses;
        } else {
            smsc_log_debug( slog, "hit: %s, session key: %s", sk.toString().c_str(), v->getKey().toString().c_str() );
            if ( v->getKey() != sk ) {
                
                fprintf( stderr, "WARNING: different key found %s != %s\n",
                         sk.toString().c_str(), v->getKey().toString().c_str() );
                smsc_log_error( slog, "WARNING: different key found %s != %s",
                                sk.toString().c_str(), v->getKey().toString().c_str() );
                delete store->purge( sk );
                v = new Session( sk );
                store->set( sk, v );
            }
        }


        if ( random() % 100 < cfg.flushprob ) {

            // flushing
            smsc_log_debug( slog, "STARTING FLUSHING CACHE" );
            ++cfg.totalflushes;
            cfg.totalflushsessions += store->flush();
            smsc_log_debug( slog, "FLUSH FINISHED, STARTING CLEANING" );

            // remove all cached objects
            // 10 % chance of clean objects even from disk
            const bool fromdisk = ( random() % 100 < cfg.cleanprob );
            if ( fromdisk ) {
                ++cfg.totalcleans;
                smsc_log_debug( slog, "CLEAN ALSO WILL AFFECT DISK" );
            }

            CSessionKey k;
            Session* dummy;
            for ( SessionStorage::iterator_type j( store->begin() ); j.next( k, dummy ); ) {
                smsc_log_debug( slog, "delete: %s", k.toString().c_str() );
                if ( ! fromdisk ) {
                    dummy = store->purge( k );
                } else {
                    dummy = store->release( k );
                    ++cfg.totalcleansessions;
                }
                delete dummy;
            }
            smsc_log_debug( slog, "CACHE CLEANED" );

        }

    }

    store->flush();

    // clean all items from cache
    {
        CSessionKey k;
        Session* dummy;
        for ( SessionStorage::iterator_type j( store->begin() ); j.next( k, dummy ); ) {
            smsc_log_debug( slog, "delete: %s", k.toString().c_str() );
            dummy = store->purge( k );
            delete dummy;
        }
    }

    // printout the statistics
    printf( "total cache hits    : %d\n", store->hitcount() );
    printf( "total disk hits     : %d\n", cfg.totalpasses - store->hitcount() - cfg.totalmisses );
    printf( "total accesses      : %d\n", cfg.totalpasses );
    printf( "total misses        : %d\n", cfg.totalmisses );
    printf( "total flushes       : %d\n", cfg.totalflushes );
    printf( "total flushed items : %d\n", cfg.totalflushsessions );
    printf( "total cleans        : %d\n", cfg.totalcleans );
    printf( "total cleaned items : %d\n", cfg.totalcleansessions );
    printf( "cache size          : %d\n", cfg.cachesize );
    printf( "number interval     : %d\n", cfg.interval );

    fprintf( stderr, "STARTING STORAGE CHECK\n" );
    smsc_log_debug( slog, "STARTING STORAGE CHECK" );
    unsigned int postcount = checkStorage( cfg, store, ok );
    fprintf( stderr, "STORAGE CHECK FINISHED\n" );
    smsc_log_debug( slog, "STORAGE CHECK FINISHED" );
    printf( "items on disk (pre) : %d\n", precount );
    printf( "items on disk (post): %d\n", postcount );
    if ( ! ok ) return 1;

    return 0;
}



unsigned int checkStorage( const Config& cfg, SessionStorage* store, bool& ok )
{
    smsc::logger::Logger* slog = cfg.slog;

    CSessionKey prevk;
    CSessionKey k;
    Session s;
    unsigned int count = 0;
    DiskStorage::index_type idx;
    ok = true;

    for ( DiskStorage::iterator_type i = store->dataBegin();
          i.next(k,idx,s);
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
            smsc_log_error( slog, "WARNING: keys out of order: %s >= %s\n",
                            prevk.toString().c_str(),
                            k.toString().c_str() );
            ok = false;
        }
        prevk = k;
        // if ( count % 100 == 0 ) {
        // fprintf( stderr, "store check pass #%d, key=%s\n", count, k.toString().c_str() );
        smsc_log_debug( slog, "store check #%d, key=%s, hash=%u, idx=%llx",
                        count,
                        k.toString().c_str(),
                        CSessionKey::CalcHash(k),
                        static_cast<unsigned long long>( idx ) );
        // }
        if ( k != s.getKey() ) {
            fprintf( stderr, "WARNING: key mismatch: %s(%u) != %s(%u)\n",
                     k.toString().c_str(),
                     CSessionKey::CalcHash(k),
                     s.getKey().toString().c_str(),
                     CSessionKey::CalcHash(s.getKey()) );
            smsc_log_error( slog, "WARNING: key mismatch: %s(%u) != %s(%u)\n",
                            k.toString().c_str(),
                            CSessionKey::CalcHash(k),
                            s.getKey().toString().c_str(),
                            CSessionKey::CalcHash(s.getKey()) );
            ok = false;
        }
    }
    return count;
}
