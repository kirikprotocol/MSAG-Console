#ifndef SMSC_WSME_PROCESSOR
#define SMSC_WSME_PROCESSOR

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <map>

#include <util/debug.h>
#include <util/Logger.h>
#include <util/Exception.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/IntHash.hpp>

#include <util/config/Config.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <db/DataSource.h>

#include "WSmeExceptions.h"
#include "WSmeAdmin.h"

namespace smsc { namespace wsme
{
    using namespace smsc::db;
    using namespace smsc::core::buffers;
                              
    using smsc::util::Logger;
    using smsc::util::Exception;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    using smsc::core::threads::Thread;
    using smsc::core::synchronization::Event;
    using smsc::core::synchronization::Mutex;

    class VisitorManager
    {
    private:

        log4cpp::Category&  log;
        DataSource&         ds;

        Mutex       visitorsLock;
        Hash<bool>  visitors;

        void loadUpVisitors()
            throw (InitException);
        
    public:

        VisitorManager(DataSource& _ds, ConfigView* config)
            throw (InitException);
        virtual ~VisitorManager();

        bool isVisitor(const std::string msisdn)
            throw (ProcessException);
        void addVisitor(const std::string msisdn)
            throw (ProcessException);
        void removeVisitor(const std::string msisdn)
            throw (ProcessException);
    };
    
    class LangManager
    {
    private:
        
        log4cpp::Category&  log;
        DataSource&         ds;
        
        std::string       defaultLang;
        Mutex             langsLock;
        Hash<std::string> langs; // Contains langs by mask
        
        void loadUpLangs()
            throw (InitException);

    public:

        LangManager(DataSource& _ds, ConfigView* config)
            throw (InitException);
        virtual ~LangManager();

        std::string getLangCode(const std::string msisdn)
            throw (ProcessException);
        std::string getDefaultLang();

        void addLang(const std::string mask, std::string lang)
            throw (ProcessException);
        void removeLang(const std::string mask)
            throw (ProcessException);
    };
    
    struct AdIdManager
    {
        AdIdManager() {};
        virtual ~AdIdManager() {};

        virtual int getFirstId() = 0;
        virtual int getNextId(int id) = 0;
    };

    typedef Hash<std::string>       AdsVal;
    typedef std::map<int, AdsVal*>  AdsMap;
    typedef std::pair<int, AdsVal*> AdsPair;
    typedef AdsMap::iterator        AdsIterator;

    class AdRepository : public AdIdManager
    {
    private:
        
        log4cpp::Category&  log;
        DataSource&         ds;

        Mutex  adsLock;
        AdsMap ads;

        void loadUpAds()
            throw (InitException);
    
    public:

        AdRepository(DataSource& _ds, ConfigView* config)
            throw(ConfigException, InitException);
        virtual ~AdRepository();

        virtual int getFirstId();
        virtual int getNextId(int id);
        
        bool getAd(int id, const std::string lang, std::string& ad)
            throw (ProcessException);
        
        void addAd(int id, const std::string lang, std::string ad)
            throw (ProcessException);
        void removeAd(int id, const std::string lang)
            throw (ProcessException);
    };
    
    class AdHistory : public Thread
    {
    private:
        
        log4cpp::Category&  log;
        DataSource&         ds;
        
        AdIdManager&        idManager;

        time_t  historyAge, messageLife, cleanupPeriod;

        Event   job, exit, exited;
        Mutex   startLock;
        bool    bStarted;
        
        void cleanup()
            throw (ProcessException);
    
    public:

        AdHistory(DataSource& _ds, ConfigView* config, AdIdManager& idman)
            throw(ConfigException, InitException);
        virtual ~AdHistory();

        int  Execute();
        void Start();
        void Stop();

        bool getId(const std::string msisdn, int& id)
            throw (ProcessException);
        void respondAd(const std::string msisdn, 
                       const std::string msgid, bool responded)
            throw (ProcessException);
        void receiptAd(const std::string msgid, bool receipted) 
            throw (ProcessException);
    };
    
    
    class AdManager
    {
    private:

        log4cpp::Category&  log;
        
    protected:

        Mutex           initLock;
        DataSource&     ds;

        AdHistory*      history;
        AdRepository*   repository;
        
    public:

        AdManager(DataSource& _ds, ConfigView* config)
            throw(ConfigException, InitException);
        virtual ~AdManager();

        bool getAd(const std::string msisdn, const std::string lang,
                   std::string& ad) 
            throw (ProcessException);
        void respondAd(const std::string msisdn, 
                       const std::string msgid, bool responded)
            throw (ProcessException);
        void receiptAd(const std::string msgid, bool receipted) 
            throw (ProcessException);

        void addAd(int id, const std::string lang, std::string ad)
            throw (ProcessException);
        void removeAd(int id, const std::string lang)
            throw (ProcessException);
    };
    
    class WSmeProcessor : public WSmeAdmin
    {
    private:
        
        log4cpp::Category   &log;
        
    protected:
        
        DataSource*         ds;

        VisitorManager*     visitorManager;
        LangManager*        langManager;
        AdManager*          adManager;
    
        int     protocolId;
        char*   svcType;
        
        void init(ConfigView* config)
            throw(ConfigException);

    public:

        WSmeProcessor(ConfigView* config) 
            throw(ConfigException, InitException);
        virtual ~WSmeProcessor();
        
        const char* getSvcType() { return (svcType) ? svcType:"WSme"; };
        int getProtocolId() { return protocolId; };
        
        /* ------------------------ Smsc interface ------------------------ */ 

        virtual bool processNotification(const std::string msisdn, 
                                         std::string& out) 
            throw (ProcessException);
        virtual void processResponce(const std::string msisdn, 
                                     const std::string msgid, bool responded)
            throw (ProcessException);

        virtual void processReceipt(const std::string msgid, bool receipted)
            throw (ProcessException);
        
        /* ------------------------ Admin interface ------------------------ */ 

        virtual void addVisitor(const std::string msisdn)
            throw (ProcessException);
        virtual void removeVisitor(const std::string msisdn)
            throw (ProcessException);
        
        virtual void addLang(const std::string mask, std::string lang)
            throw (ProcessException);
        virtual void removeLang(const std::string mask)
            throw (ProcessException);
       
        virtual void addAd(int id, const std::string lang, std::string ad)
            throw (ProcessException);
        virtual void removeAd(int id, const std::string lang)
            throw (ProcessException);
    };

}}

extern bool compareMaskAndAddress(const std::string mask, 
                                  const std::string addr);

#endif // SMSC_WSME_PROCESSOR


