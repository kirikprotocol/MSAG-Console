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

#include <util/debug.h>
#include <util/Logger.h>
#include <util/Exception.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>

#include <util/config/Config.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <db/DataSource.h>

namespace smsc { namespace wsme
{
    using namespace smsc::db;
    using namespace smsc::core::buffers;
                              
    using smsc::util::Logger;
    using smsc::util::Exception;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    class InitException : public Exception
    {
    public:

        InitException(const char* fmt,...)
            : Exception() 
        { 
            SMSC_UTIL_EX_FILL(fmt);
        };
        InitException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~InitException() throw() {};
    };
    
    class ProcessException : public Exception
    {
    public:

        ProcessException(const char* fmt,...)
            : Exception() 
        { 
            SMSC_UTIL_EX_FILL(fmt);
        };
        ProcessException(const Exception& exc)
            : Exception(exc.what()) {};
        virtual ~ProcessException() throw() {};
    };

    class VisitorManager
    {
    private:

        DataSource& ds;

        Mutex               visitorsLock;
        Array<std::string>  visitors;

        void loadUpVisitors()
            throw (InitException);
        
    public:

        VisitorManager(DataSource& _ds)
            throw (InitException);
        virtual ~VisitorManager();

        bool isVisitor(const std::string msisdn)
            throw (ProcessException);
    };
    
    class LangManager
    {
    private:
        
        struct LangInfo
        {
            std::string mask;
            std::string lang;

            LangInfo() {};
            LangInfo(std::string _mask, std::string _lang) 
                : mask(_mask), lang(_lang) {};
        };
        
        DataSource& ds;
        
        Mutex           langsLock;
        Array<LangInfo> langs;
        
        void loadUpLangs()
            throw (InitException);

    public:

        LangManager(DataSource& _ds)
            throw (InitException);
        virtual ~LangManager();

        bool getLangCode(const std::string msisdn, std::string& lang)
            throw (ProcessException);
    };
    
    struct AdIdManager
    {
        AdIdManager() {};
        virtual ~AdIdManager() {};

        virtual int getFirstId() = 0;
        virtual int getNextId(int id) = 0;
    };

    class AdRepository : public AdIdManager
    {
    private:
        
        log4cpp::Category&  log;
        DataSource&         ds;

        int maxAdId;
        void loadMaxAdId()
            throw(InitException);
    
    public:

        AdRepository(DataSource& _ds, ConfigView* config)
            throw(ConfigException, InitException);
        virtual ~AdRepository();

        virtual int getFirstId();
        virtual int getNextId(int id);
        
        bool getAd(int id, const std::string lang, bool isLang, std::string& ad)
            throw (ProcessException);
    };
    
    class AdHistory
    {
    private:
        
        log4cpp::Category&  log;
        DataSource&         ds;
        
        AdIdManager&        idManager;

        int keepPeriod, lifePeriod;
    
    public:

        AdHistory(DataSource& _ds, ConfigView* config, AdIdManager& idman)
            throw(ConfigException, InitException);
        virtual ~AdHistory();

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

        bool getAd(const std::string msisdn, 
                   const std::string lang, bool isLang,
                   std::string& ad) 
            throw (ProcessException);
        void respondAd(const std::string msisdn, 
                       const std::string msgid, bool responded)
            throw (ProcessException);
        void receiptAd(const std::string msgid, bool receipted) 
            throw (ProcessException);
    };
    
    class WSmeProcessor
    {
    private:
        
        log4cpp::Category   &log;
        
    protected:
        
        DataSource*         ds;

        VisitorManager*     visitorManager;
        LangManager*        langManager;
        AdManager*          adManager;
    
        void init(ConfigView* config)
            throw(ConfigException);

    public:

        WSmeProcessor(ConfigView* config) 
            throw(ConfigException, InitException);
        virtual ~WSmeProcessor();
        
        virtual bool processNotification(const std::string msisdn, 
                                         std::string& out) 
            throw (ProcessException);
        virtual void processResponce(const std::string msisdn, 
                                     const std::string msgid, bool responded)
            throw (ProcessException);

        virtual void processReceipt(const std::string msgid, bool receipted)
            throw (ProcessException);
    };

}}

#endif // SMSC_WSME_PROCESSOR


