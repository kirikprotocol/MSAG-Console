#ifndef FILE_STORAGE_DECLARATIONS
#define FILE_STORAGE_DECLARATIONS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <string>

#include <util/config/Manager.h>
#include <logger/Logger.h>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/TmpBuf.hpp>

#include "StoreExceptions.h"

namespace smsc { namespace store
{
    using smsc::logger::Logger;
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigException;

    using namespace smsc::sms;
    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;
    
    extern const char* SMSC_LAST_BILLING_FILE_EXTENSION;
    extern const char* SMSC_PREV_BILLING_FILE_EXTENSION;
    extern const char* SMSC_LAST_ARCHIVE_FILE_EXTENSION;
    extern const char* SMSC_PREV_ARCHIVE_FILE_EXTENSION;
    extern const char* SMSC_PERSIST_FILE_NAME_PATTERN;
    extern const char* SMSC_PERSIST_DIR_NAME_PATTERN;

    class FileStorage
    {
    public:

        static bool createDir(std::string dir);
        static void deleteFile(std::string location, std::string fileName);
        static void rollErrorFile(std::string location, std::string fileName);
        static void rollFileExtension(std::string location, const char* fileName, bool bill=true);
        
        static void findEntries(std::string location, Array<std::string>& entries, 
                                bool files=false, const char* ext=0);
        static void findDirs (std::string location, Array<std::string>& dirs);
        static void findFiles(std::string location, const char* ext, Array<std::string>& files);
        
        static uint64_t toHostOrder(uint64_t value);
        static uint64_t toNetworkOrder(uint64_t value);
        
        bool read(void* data, size_t size);
        void write(const void* data, size_t size);
        void flush();
        void close();
        
        void getPos(fpos_t* pos);
        void setPos(const fpos_t* pos);

        void save(SMSId id, SMS& sms, fpos_t* pos=0);
        bool load(SMSId& id, SMS& sms, const fpos_t* pos=0);
        
        virtual ~FileStorage() {
            close();
        };

    protected:
        
        smsc::logger::Logger *log;

        FILE*           storageFile;
        Mutex           storageFileLock;
        std::string     storageLocation;
        
        FileStorage() : 
            log(Logger::getInstance("smsc.store.FileStorage")), storageFile(0) {};
    };

    class RollingStorage : public FileStorage
    {
    protected:

        uint32_t        storageInterval;
        char            storageFileName[256];

        RollingStorage() : FileStorage(), storageInterval(0) {};

    public:

        virtual ~RollingStorage() {};
        
        void init(Manager& config, bool bill);
        bool create(bool bill);
        void roll(bool bill);
        
        inline uint32_t getStorageInterval() {
            return storageInterval;
        };
        inline const char* getStorageLocation() {
            return storageLocation.c_str();
        };
    };

    class BillingStorage : public RollingStorage
    {
    protected:

        void create();

    public:

        BillingStorage() : RollingStorage() {};
        virtual ~BillingStorage() {};

        void init(Manager& config) {
            RollingStorage::init(config, true);
        };
        void roll() {
            RollingStorage::roll(true);
        };

        void createRecord(SMSId id, SMS& sms);
    };
    
    class ArchiveStorage : public RollingStorage
    {
    protected:

        void create();
        
    public:
        
        ArchiveStorage() : RollingStorage() {};
        virtual ~ArchiveStorage() {};

        void init(Manager& config) {
            RollingStorage::init(config, false);
        };
        void roll() {
            RollingStorage::roll(false);
        };

        void createRecord(SMSId id, SMS& sms);
    };

    class PersistentStorage : public FileStorage
    {
    protected:
        
        std::string storageFileName;

        bool create(bool create);
        void open(bool read);

    public:

        PersistentStorage(std::string location, std::string filename) 
            : FileStorage(), storageFileName(filename) { storageLocation = location; };
        virtual ~PersistentStorage() {};

        bool readRecord(SMSId& id, SMS& sms, const fpos_t* pos=0);
        void writeRecord(SMSId id, SMS& sms, fpos_t* pos=0);
    };
    

}}

#endif // FILE_STORAGE_DECLARATIONS

