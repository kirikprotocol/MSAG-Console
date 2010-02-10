#include <cassert>
#include <string>
#include <list>
#include <cstdlib>  // random
#include <string.h> // memcpy
#include <memory>
#include <time.h>   // nanosleep
#include "RBTreeIndexStorage.h"
#include "HashedMemoryCache.h"
#include "ArrayedMemoryCache.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/synchronization/MutexGuard.hpp"
#include "core/threads/Thread.hpp"
#include "logger/Logger.h"
#include "scag/util/io/Serializer.h"
#include "StorageIface.h"
#include "StorageNumbering.h"
#include "scag/util/io/GlossaryBase.h"
#include "scag/pvss/profile/AbntAddr.hpp"

// please comment out for BHS
//#define USEPAGEFILE
#ifdef USEPAGEFILE
#include "PageFileDiskStorage.h"
#else
#include "BHDiskStorage2.h"
#endif

// please comment out for single storage
// #define USECOMPOSITE
#ifdef USECOMPOSITE
#include "CompositeDiskStorage.h"
#endif

// for __require__
#ifndef NOLOGGERPLEASE
#include "util/debug.h"
#else
#define __require__(x)
#endif


using namespace scag::util::storage;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::MutexGuard;


namespace {
#ifdef USEPAGEFILE    
    const std::string storagesuffix("-pgf");
#else
    const std::string storagesuffix("-bhs");
#endif
}


#if 0
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


struct CSessionKey 
{

    CSessionKey() {}
    CSessionKey( const Address& a ) : abonentAddr(a) {}

    inline bool operator ==(const CSessionKey& sk) const
    {
        return ( this->abonentAddr == sk.abonentAddr );
    }

    inline bool operator !=(const CSessionKey& sk) const
    {
        return ! ( this->abonentAddr == sk.abonentAddr );
    }

    bool operator < ( const CSessionKey& sk ) const
    {
        return ( abonentAddr < sk.abonentAddr );
        // if ( sk.abonentAddr < abonentAddr ) return false;
        // if ( USR < sk.USR ) return true;
        // return false;
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

Serializer& operator << ( Serializer& s, const CSessionKey& sk )
{
    s << sk.abonentAddr.length << sk.abonentAddr.type << sk.abonentAddr.plan;
    s.writeAsIs(MAX_ADDRESS_VALUE_LENGTH+1,sk.abonentAddr.value);
    return s;
}

Deserializer& operator >> ( Deserializer& s, CSessionKey& sk )
{
    s >> sk.abonentAddr.length >> sk.abonentAddr.type >> sk.abonentAddr.plan;
    strncpy(sk.abonentAddr.value,s.readAsIs(MAX_ADDRESS_VALUE_LENGTH+1),MAX_ADDRESS_VALUE_LENGTH);
    sk.abonentAddr.value[MAX_ADDRESS_VALUE_LENGTH] = '\0';
    return s;
}

#endif // if 0

using scag2::util::io::Serializer;
using scag2::util::io::Deserializer;

typedef scag2::pvss::AbntAddr CSessionKey;

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

        const std::string& toString() const { return sessionKey.toString() + ":" + somedata; }

/*
#ifndef USEPAGEFILE
        // these two methods are necessary for BlocksHSStorage
        void Serialize( SerialBuffer& buf, bool = false, GlossaryBase* g = NULL ) const;
        void Deserialize( SerialBuffer& buf, bool = false, GlossaryBase* g = NULL );
#endif
 */
    
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
    pfb << sessionKey << uint32_t(lastAccessTime);
    pfb << somedata;
}


void Session::deserialize( Deserializer& pfb )
{
    uint32_t tm;
    CSessionKey sk;
    pfb >> sk >> tm;
    pfb >> somedata;
    sessionKey = sk;
    lastAccessTime = time_t(tm);
}


/*
#ifndef USEPAGEFILE
void Session::Serialize( SerialBuffer& buf, bool, GlossaryBase* g ) const
{
    std::vector< unsigned char > v;
    Serializer s( v );
    serialize( s );
    buf.blkcpy( reinterpret_cast<const char*>(&(v[0])), v.size() );
}

void Session::Deserialize( SerialBuffer& buf, bool, GlossaryBase* g )
{
    Deserializer d( reinterpret_cast<const unsigned char*>(buf.c_ptr()), buf.length() );
    deserialize( d );
}
#endif
 */

    void Session::init()
    {
        const std::string s = "0123456789abcdefghijklmnopqrstuvwxyz";
        const std::string ss = s + s + s + s + s + s + s + s;
        somedata = "sessionData[" + ss + ss + ss + ss + ss + "]";
        // somedata = "sessionData[" + s + "]";
    }


Serializer& operator << ( Serializer& s, const Session& ss ) {
    ss.serialize( s );
    return s;
}

Deserializer& operator >> ( Deserializer& s, Session& ss ) {
    ss.deserialize( s );
    return s;
}



#ifdef USEPAGEFILE
class DelayedPageFile : public smsc::core::buffers::PageFile
{
public:
    DelayedPageFile( unsigned delayusec ) :
    smsc::core::buffers::PageFile(), delay_(delayusec) {}
    ~DelayedPageFile() {
        // we have to delete file event handler
        delete file.GetEventHandler();
    }
    void Create( const std::string& fn, int psz, int prealloc = 1024 )
    {
        smsc::core::buffers::PageFile::Create( fn, psz, prealloc );
        setFEH();
    }
    void Open( const std::string& fn )
    {
        PageFile::Open( fn );
        setFEH();
    }
protected:
    class WriteDelayEventHandler : public smsc::core::buffers::FileEventHandler 
    {
    public:
        WriteDelayEventHandler( unsigned delayusec ) {
            delay_.tv_sec = delayusec/1000000;
            delay_.tv_nsec = (delayusec%1000000)*1000;
        }
        virtual ~WriteDelayEventHandler() {}
        virtual void onOpen( int, const char* ) {}
        virtual void onRead( const void*, size_t ) {}
        virtual void onWrite( const void*, size_t ) {
            // make a delay
            nanosleep( &delay_, NULL );
        }
        virtual void onSeek( int, int64_t ) {}
        virtual void onRename( const char* ) {}
    private:
        struct timespec delay_;
    };

    void setFEH() {
        if ( delay_ > 0 ) {
            smsc::core::buffers::FileEventHandler* prev = file.GetEventHandler();
            file.SetEventHandler( new WriteDelayEventHandler( delay_ ) );
            delete prev;
        }
    }

protected:
    unsigned delay_;  // in microseconds
};
#endif


#ifdef USEPAGEFILE
//typedef HashedMemoryCache< CSessionKey, Session > MemStorage;
typedef ArrayedMemoryCache< CSessionKey, Session > MemStorage;
typedef PageFileDiskStorage< CSessionKey, Session, DelayedPageFile > DiskDataStorage;
#else
// typedef HashedMemoryCache< CSessionKey, Session, DataBlockBackupTypeJuggling > MemStorage;
typedef ArrayedMemoryCache< CSessionKey, Session, DataBlockBackupTypeJuggling2 > MemStorage;
typedef BHDiskStorage2< CSessionKey, Session > DiskDataStorage;
#endif
typedef RBTreeIndexStorage< CSessionKey, DiskDataStorage::index_type > DiskIndexStorage;
#ifdef USECOMPOSITE
typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > EltDiskStorage;
typedef CompositeDiskStorage< EltDiskStorage > DiskStorage;
#else
typedef IndexedStorage< DiskIndexStorage, DiskDataStorage > DiskStorage;
#endif
typedef CachedDiskStorage< MemStorage, DiskStorage > SessionStorage;


struct Config {

    std::string helpMessage() const
    {
        char buf[10240];
        snprintf
            (buf,sizeof(buf),
             "Usage: testrb\n"
             "   The following envvars allowed:\n"
             "mynode       specifies which node we work with among 5 (%u)\n"
             "initrand     what is the initial random number (%u)\n"
             "nocheck      =1 to skip initial check (%u)\n"
             "indexgrowth  what is the size of the index file (%u)\n"
             "cachesize    what is the size of cache (%u)\n"
             "pagesize     what is the size of block in datafile (%u)\n"
             "preallocate  what is the size of one data file in blocks (%u)\n"
             "interval     what is the interval of abonents (%u)\n"
             "pfdelay      the delay in pagefile, msec? (%u)\n"
             "totalpasses  how many passes should be done (%u)\n"
             "flushprob    the probability of flush (%u)\n"
             "cleanprob    the probability of clean (%u)\n"
             "minkilltime  the time after which kill may be issued (%u)\n"
             "maxkilltime  the time before which kill may be issued (%u)\n",
             unsigned(mynode),
             unsigned(initrand),
             unsigned(nocheck),
             unsigned(indexgrowth),
             unsigned(cachesize),
             unsigned(pagesize),
             unsigned(preallocate),
             unsigned(interval),
             unsigned(pfdelay),
             unsigned(totalpasses),
             unsigned(flushprob),
             unsigned(cleanprob),
             unsigned(minkilltime),
             unsigned(maxkilltime) 
             );
        return buf;
    }


    Config( smsc::logger::Logger* slogg ) {

        slog = slogg;

        StorageNumbering::setInstance( /* nodes */ 5 );

        mynode = 0;
        if ( getenv("mynode") ) {
            mynode = strtoul(getenv("mynode"), NULL, 10 );
        }
        assert( mynode < StorageNumbering::instance().nodes() );

        initrand = 0;
        if ( getenv("initrand") ) {
            initrand = strtol(getenv("initrand"), NULL, 10 );
        }
        srandom( initrand );
        for ( size_t i = 0; i < 3; ++i )
            smsc_log_info( slog, "a random: %ld", random() );

        storagename = "sessions";
        storagepath = "./sessions";

        nocheck = 0;
        if ( getenv("nocheck") ) {
            nocheck = strtoul(getenv("nocheck"), NULL, 10 );
        }

        indexgrowth = 1000;
        if ( getenv("indexgrowth") ) {
            indexgrowth = strtoul(getenv("indexgrowth"), NULL, 10 );
        }
        cachesize = 1000;
        if ( getenv("cachesize") ) {
            cachesize = strtoul(getenv("cachesize"), NULL, 10 );
        }
        pagesize = 256;
        if ( getenv("pagesize") ) {
            pagesize = strtoul(getenv("pagesize"), NULL, 10 );
        }
        preallocate = 100000;
        if ( getenv("preallocate") ) {
            preallocate = strtoul(getenv("preallocate"), NULL, 10 );
        }
        interval = 1000000;
        if ( getenv("interval") ) {
            interval = strtoul(getenv("interval"), NULL, 10 );
        }
        pfdelay = 0;
        if ( getenv("pfdelay") ) {
            pfdelay = strtoul(getenv("pfdelay"), NULL, 10 );
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
        minkilltime = 1;
        if ( getenv("minkilltime") ) {
            minkilltime = strtoul(getenv("minkilltime"), NULL, 10 );
        }
        maxkilltime = 0;
        if ( getenv("maxkilltime") ) {
            maxkilltime = strtoul(getenv("maxkilltime"), NULL, 10 );
        }
        if ( maxkilltime < minkilltime ) {
            maxkilltime = minkilltime;
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
    unsigned    mynode;
    unsigned    nocheck;
    unsigned    indexgrowth;
    unsigned    cachesize;
    unsigned    pagesize;
    unsigned    preallocate;
    unsigned    interval;
    unsigned    pfdelay;
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
    mainid_(mainid), wait_(sectowait) {}
    virtual ~SelfKiller() {
        // main has finished early
        printf( "main has finished early\n" );
        MutexGuard mg(mtx_);
        if ( running ) {
            printf( "thread is running\n" );
            running = false;
            Kill(SIGUSR1);
            mtx_.wait();
        }
        printf( "leaving dtor\n" );
    }
    virtual int Execute();
private:
    EventMonitor mtx_;
    bool running;
    int  mainid_;
    int  wait_;
};


int SelfKiller::Execute()
{
    {
        printf( "set running state\n" );
        MutexGuard mg(mtx_);
        running = true;
    }
    struct timespec req;
    req.tv_sec = wait_;
    req.tv_nsec = 0;
    fprintf( stderr, "going to nanosleep for %d seconds\n", wait_ );
    int ret;
    if ( ( ret = nanosleep( &req, NULL ) == EINTR ) ) {
        printf( "nanosleep has been killed\n" );
        return 1;
    }
    printf( "awoken from nanosleep\n" );

    MutexGuard mg(mtx_);
    if ( running ) {
        printf( "going to kill main\n" );
        pthread_kill( mainid_, SIGKILL );
    }
    return 0;
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

    if ( argc > 1 ) {
        std::string sarg(argv[1]);
        if ( sarg == "--help" ) {
            fprintf( stderr, "%s\n", cfg.helpMessage().c_str() );
        }
        exit(-1);
    }

    DataFileManager dataFileManager(4,400);
    std::auto_ptr< SessionStorage > store;

    do {

        std::auto_ptr< DiskStorage > ds;
        std::auto_ptr< MemStorage > ms;

#ifdef USECOMPOSITE
        ds.reset( new DiskStorage(smsc::logger::Logger::getInstance("diskcompos")) );
        smsc_log_debug( slog, "disk storage is created" );

        for ( unsigned idx = 0; idx < StorageNumbering::instance().storages(); ++idx ) {

            if ( cfg.mynode != StorageNumbering::instance().node(idx) ) continue;

            char buf[10];
            snprintf( buf, sizeof(buf), "%03u", idx );
            const std::string idxstr(buf);
#else
            unsigned idx = 0;
            const std::string idxstr;
#endif
            
            // std::auto_ptr< DelayedPageFile > pf;
            std::auto_ptr< DiskDataStorage::storage_type > pf;
            std::auto_ptr< DiskDataStorage > dds;
            std::auto_ptr< DiskIndexStorage > dis;

            // --- setup is here
            dis.reset( new DiskIndexStorage( cfg.storagename + storagesuffix + idxstr,
                                             cfg.storagepath,
                                             cfg.indexgrowth,
                                             false,
                                             smsc::logger::Logger::getInstance("diskindex") ));
            smsc_log_debug( slog, "data index %u storage is created", idx );
            // return testDiskIndexStorage( cfg, dis.get() );


#ifdef USEPAGEFILE
            const std::string fn( cfg.storagepath + "/" + cfg.storagename + storagesuffix + idxstr + "-data" );
            pf.reset( new DelayedPageFile(cfg.pfdelay) );
            try {
                pf->Open( fn );
            } catch (...) {
                pf->Create( fn, cfg.pagesize, cfg.preallocate );
            }
#else
            pf.reset( new DiskDataStorage::storage_type(dataFileManager,
                                                        smsc::logger::Logger::getInstance("diskstore")) );
            int ret = -1;
            const std::string fn( cfg.storagename + storagesuffix + idxstr + "-data" );
            try {
                // ret = pf->Open( fn, cfg.storagepath );
                ret = pf->open( fn, cfg.storagepath );
            } catch (...) {
                ret = -1;
            }

            if ( ret < 0 ) ret = pf->create( fn,
                                             cfg.storagepath,
                                             cfg.preallocate,
                                             cfg.pagesize );
            if ( ret < 0 ) {
                smsc_log_fatal(slog,"cannot create storage %u",idx);
                exit(-1);
            }
#endif
            smsc_log_debug( slog, "storage %u is created", idx );


            dds.reset( new DiskDataStorage( pf.release(),
                                            0, // glossary
                                            smsc::logger::Logger::getInstance("diskdata") ) );
            smsc_log_debug( slog, "data disk storage %u is created" );

            
#ifdef USECOMPOSITE
            std::auto_ptr< EltDiskStorage > 
                eds( new EltDiskStorage( dis.release(), dds.release() ) );
            smsc_log_debug( slog, "elt disk storage %u is assembled" );

            ds->addStorage( idx, eds.release() );
        }
#else
        ds.reset( new DiskStorage(dis.release(), dds.release()) );
#endif
        smsc_log_debug( slog, "disk storage is assembled" );

        ms.reset( new MemStorage( smsc::logger::Logger::getInstance("cache"),
                                  cfg.cachesize ) );
        smsc_log_debug( slog, "memory storage is created" );

        store.reset( new SessionStorage( ms.release(), ds.release(),
                                         smsc::logger::Logger::getInstance("store" ) ) );
        smsc_log_debug( slog, "session storage is assembled" );

    } while ( false );

    if ( ! store.get() ) return 0;

    return testSessionStorage( cfg, store.get() );
}



CSessionKey genKey( const Config& cfg )
{
    uint8_t ton = random() ? 0 : 1;
    uint8_t npi = 1;
    char buf[20];
    unsigned long long number;
    do {
        number = 89130000000ULL + random() % cfg.interval;
    } while ( StorageNumbering::instance().nodeByNumber(number) != cfg.mynode );
    int len = sprintf( buf, "%011llu", number );
    return CSessionKey( len, ton, npi, buf );
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
    
    unsigned filledcount = 0;
    for ( DiskIndexStorage::iterator_type i( dis->begin() ); i.next(); ) {
        DiskIndexStorage::key_type k = i.key();
        DiskIndexStorage::index_type idx = i.idx();

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
        DiskIndexStorage::index_type j = k.getNumber(); // k.toIndex();
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
    std::auto_ptr< SelfKiller > selfkill;
    if ( cfg.minkilltime != cfg.maxkilltime ) {
        selfkill.reset( new SelfKiller
                        ( pthread_self(),
                          random() % (cfg.maxkilltime-cfg.minkilltime) + cfg.minkilltime ) );
        selfkill->Start();
    }

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
            if ( dis->setIndex( sk, sk.getNumber() ) ) {
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

    bool ok = true;
    unsigned int precount = 0;
    if ( ! cfg.nocheck ) {

        fprintf( stderr, "STARTING STORAGE CHECK\n" );
        smsc_log_debug( slog, "STARTING STORAGE CHECK" );
        precount = checkStorage( cfg, store, ok );
        fprintf( stderr, "STORAGE CHECK FINISHED\n" );
        smsc_log_debug( slog, "STORAGE CHECK FINISHED" );
        if ( ! ok ) return 1;
    }
    
    // start self killer
    std::auto_ptr< SelfKiller > selfkill;
    if ( cfg.minkilltime != cfg.maxkilltime ) {
        selfkill.reset( new SelfKiller
                        ( pthread_self(),
                          random() % (cfg.maxkilltime-cfg.minkilltime) + cfg.minkilltime ) );
        selfkill->Start();
    }

    for ( size_t i = 1; i <= cfg.totalpasses; ++i ) {

        const CSessionKey sk( genKey( cfg ) );
        Session* v = store->get( sk );
        if ( ! v ) {
            smsc_log_debug( slog, "mis: %s ... added to cache", sk.toString().c_str() );
            v = new Session( sk );
            store->set( sk, v );
            ++cfg.totalmisses;
        } else {
            smsc_log_debug( slog, "hit: %s, session key: %s", sk.toString().c_str(), v->getKey().toString().c_str() );
            if ( ! (v->getKey() == sk) ) {
                
                fprintf( stderr, "WARNING: different key found %s != %s\n",
                         sk.toString().c_str(), v->getKey().toString().c_str() );
                smsc_log_error( slog, "WARNING: different key found %s != %s",
                                sk.toString().c_str(), v->getKey().toString().c_str() );
                return 1;
            }
        }


        if ( i % 100 == 0 ) {
            fprintf( stderr, "pass #%d, inserts=%d, removes=%d, size=%d\n",
                     i, cfg.totalmisses, cfg.totalcleansessions,
                     precount+cfg.totalmisses-cfg.totalcleansessions );
            smsc_log_debug( slog, "pass #%d, inserts=%d, removes=%d, size=%d",
                            i, cfg.totalmisses, cfg.totalcleansessions,
                            precount+cfg.totalmisses-cfg.totalcleansessions );
        }

        if ( random() % 100 < cfg.flushprob ) {

            // flushing
            smsc_log_debug( slog, "STARTING FLUSHING CACHE" );
            ++cfg.totalflushes;
            cfg.totalflushsessions += store->flushAll();
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
                dummy = store->release( k, fromdisk );
                if ( fromdisk ) ++cfg.totalcleansessions;
                delete dummy;
            }
            smsc_log_debug( slog, "CACHE CLEANED" );

        }

    }

    /*
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
     */

    // printout the statistics
    {
        const Session s( genKey( cfg ) );
        std::vector< unsigned char > buf;
        Serializer ss( buf );
        s.serialize( ss );
        printf( "object size         : %u\n", buf.size() );
    }
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
    printf( "items on disk (pre) : %d\n", precount );
    printf( "items on disk (post): %d\n", precount+cfg.totalmisses-cfg.totalcleansessions );

    return 0;
}



unsigned int checkStorage( const Config& cfg, SessionStorage* store, bool& ok )
{
    smsc::logger::Logger* slog = cfg.slog;

    CSessionKey prevk;
    CSessionKey k;
    Session s;

#ifdef USEPAGEFILE
    Session& ss = s;
#else
    DataBlockBackup2< Session > ss( &s, new Serializer::Buf );
#endif

    unsigned int count = 0;
    // DiskStorage::index_type idx;
    ok = true;

    for ( DiskStorage::iterator_type i = store->dataBegin();
          i.next(k,ss);
          ) {

        ++count;

        if ( count > 5000000 ) {
            fprintf( stderr, "store check failed\n" );
            break;
        }

        /**/
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
        /**/
        if ( count % 100 == 0 ) {
            // fprintf( stderr, "store check pass #%d, key=%s\n",
            // count, k.toString().c_str() );
            smsc_log_debug( slog, "store check #%d, key=%s, hash=%u",
                            count,
                            k.toString().c_str(),
                            CSessionKey::CalcHash(k) );
            // static_cast<unsigned long long>( idx ) );
        }
        if ( !(k == s.getKey()) ) {
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

#ifndef USEPAGEFILE
    ss.value = 0; // release val
    ss.dealloc();
#endif

    smsc_log_info( slog, "total items : %d", count );

    fprintf( stderr, "total items      : %lu\n", (unsigned long)count );
    fprintf( stderr, "total indices    : %lu\n", (unsigned long)store->dataSize() );
    // fprintf( stderr, "total data items : %d\n", count );
    return count;
}
