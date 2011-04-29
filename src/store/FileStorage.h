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
#include <core/buffers/File.hpp>

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
    extern const char* SMSC_TEXT_ARCHIVE_FILE_EXTENSION;
    extern const char* SMSC_TRNS_ARCHIVE_FILE_EXTENSION;

    extern const char* SMSC_PERSIST_DIR_NAME_PATTERN;

    class FileStorage
    {
    public:

        static bool createDir(const std::string& dir);
        static void deleteFile(const std::string& fullPath);
        static void deleteFile(const std::string& location, const std::string& fileName);
        static void truncateFile(const std::string& fullPath, off_t length);
        static void truncateFile(const std::string& location, const std::string& fileName, off_t length);
        static void rollErrorFile(const std::string& location, const std::string& fileName);
        static void rollFileExtension(const std::string& location, const char* fileName);

        static void findEntries(const std::string& location, Array<std::string>& entries,
                                bool files=false, const char* ext=0);
        static void findDirs (const std::string& location, Array<std::string>& dirs);
        static void findFiles(const std::string& location, const char* ext, Array<std::string>& files);

        static uint64_t toHostOrder(uint64_t value);
        static uint64_t toNetworkOrder(uint64_t value);

        bool read(void* data, size_t size);
        void write(const void* data, size_t size);
        void flush();
        void close();

        void getPos(File::offset_type* pos);
        void setPos(const File::offset_type* pos);

        virtual ~FileStorage() {
            close();
        };

    protected:

        smsc::logger::Logger *log;

        File            storageFile;
        Mutex           storageFileLock;
        std::string     storageLocation;

        virtual void initialize(bool flag) = 0;

        void save(SMSId id, SMS& sms, File::offset_type* pos=0);
        bool load(SMSId& id, SMS& sms, File::offset_type* pos=0);
        void bill(SMSId id, SMS& sms, std::string& out);

        FileStorage() : log(Logger::getInstance("smsc.store.FileStorage")) {};
    };

    class RollingStorage : public FileStorage
    {
    protected:

        uint32_t        storageInterval;
        char            storageFileName[256];

        bool create(bool bill, bool roll);
        virtual void initialize(bool flag) = 0;

        RollingStorage() : FileStorage(), storageInterval(0) {};

    public:

        virtual ~RollingStorage() {};

        void init(const char* argStorePath, int argRollInterval);

        inline uint32_t getStorageInterval() {
            return storageInterval;
        };
        inline const char* getStorageLocation() {
            return storageLocation.c_str();
        };
    };
/*
    class BillingStorage : public RollingStorage
    {
    protected:

        virtual void initialize(bool flag) { this->create(); };
        using RollingStorage::create;
        void create(uint8_t roll=false);

    public:

        BillingStorage() : RollingStorage() {};
        virtual ~BillingStorage() {};

        using RollingStorage::init;
        void init(Manager& config)  { RollingStorage::init(config); };
        void roll();

        void createRecord(SMSId id, SMS& sms);
    };
*/
    class ArchiveStorage : public RollingStorage
    {
    protected:

        virtual void initialize(bool flag) { this->create(); };
        using RollingStorage::create;
        void create(uint8_t roll=false);

    public:

        ArchiveStorage() : RollingStorage() {};
        virtual ~ArchiveStorage() {};

        //using RollingStorage::init;
        //void init(const char* argStorePath,int argRollInterval)  { RollingStorage::init(argStorePath,argRollInterval); };
        void roll();

        void createRecord(SMSId id, SMS& sms);
    };

    class PersistentStorage : public FileStorage
    {
    protected:

        std::string storageFileName;

        virtual void initialize(bool flag) { this->open(flag); };
        bool create(bool create);
        void open(bool read);

    public:

        PersistentStorage(const std::string& location, const std::string& filename)
            : FileStorage(), storageFileName(filename) { storageLocation = location; };
        virtual ~PersistentStorage() {};

        void openRead(const File::offset_type* pos=0);
        bool readRecord(SMSId& id, SMS& sms, File::offset_type* pos=0);

        void openWrite(File::offset_type* pos=0);
        void writeRecord(SMSId id, SMS& sms, File::offset_type* pos=0);
    };

    class TextDumpStorage : public FileStorage
    {
    protected:

        std::string storageFileName;

        virtual void initialize(bool flag) {};
        bool create();
        void open();

    public:

        TextDumpStorage(const std::string& location, const std::string& filename)
            : FileStorage(), storageFileName(filename) { storageLocation = location; };
        virtual ~TextDumpStorage() {};

        void openWrite(File::offset_type* pos=0);
        void writeRecord(SMSId id, SMS& sms);
    };

    class TransactionStorage : public FileStorage
    {
    protected:

        std::string storageFileName;

        virtual void initialize(bool flag) {};
        bool open(bool create);

    public:

        TransactionStorage(const std::string& location, const std::string& filename)
            : FileStorage(), storageFileName(location+"/"+filename) { storageLocation = location; };
        TransactionStorage(const std::string& fullFileName)
            : FileStorage(), storageFileName(fullFileName) {};
        virtual ~TransactionStorage() {};

        bool getTransactionData(File::offset_type* pos);
        void setTransactionData(const File::offset_type* pos);
    };

}}

#endif // FILE_STORAGE_DECLARATIONS

