#ifndef ARCHIVE_PROCESSOR_DECLARATIONS
#define ARCHIVE_PROCESSOR_DECLARATIONS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <string>

#include <logger/Logger.h>

#include <util/config/Config.h>
#include <util/config/ConfigView.h>
#include <util/config/ConfigException.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/threads/ThreadPool.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/Array.hpp>
#include <core/network/Socket.hpp>

#include "DaemonCommunicator.h"
#include "StoreExceptions.h"

#include "index/SmsIndex.hpp"

namespace smsc { namespace store
{
    using smsc::logger::Logger;

    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;

    using namespace smsc::core::synchronization;
    using namespace smsc::core::threads;
    using namespace smsc::core::buffers;
    using namespace smsc::store::index;

    using smsc::core::network::Socket;

    struct DirEntry
    {
        std::string dir;
        uint64_t   code;

        DirEntry(std::string _dir="", uint64_t _code=0) : dir(_dir), code(_code) {};
    };

    class ArchiveProcessor;

    class Query : public ThreadedTask
    {
    friend class ArchiveProcessor;
    private:

        static Mutex        readLock;
        static EventMonitor writeMonitor;
        static int          activeCounter;

        struct ProcessQueryGuard {
            ProcessQueryGuard();
            ~ProcessQueryGuard();
        };
        struct ProcessArchiveGuard {
            ProcessArchiveGuard();
            ~ProcessArchiveGuard();
        };

        smsc::logger::Logger*   log;
        ArchiveProcessor*       processor;
        Socket*                 socket;

        int32_t                 messagesToSend;

    friend class ProcessQueryGuard;
    friend class ProcessArchiveGuard;

        bool prepareIndex(QueryMessage* query, Array<Param>& index);

        bool checkMessage(QueryMessage* query, SMSId id, SMS& sms);
        bool sendMessage(DaemonCommunicator& communicator, SMSId id, SMS& sms);

        void findDirsByQuery(QueryMessage* query, const std::string& location,
                             Array<DirEntry>& dirs);
        void findFilesByQuery(QueryMessage* query, const std::string& location,
                              uint64_t dirCode, Array<uint8_t>& files);

    public:

        Query(ArchiveProcessor* processor, Socket *socket);
        virtual ~Query();

        virtual int Execute();
        virtual const char* taskName() {
            return "ArchiveQueryTask";
        };
    };

    class ArchiveProcessor : public Thread
    {
    private:

        smsc::logger::Logger*     log;

        Hash<std::string>   locations;
        std::string         baseDirectory, textDirectory;
        Mutex               locationsLock, directoriesLock;

        ThreadPool      queriesPool;

        Socket      serverSocket;
        Event       exited;
        bool        bStarted, bNeedExit;
        Mutex       startLock;

        SmsIndex*   indexator;

        bool        bTransactionOpen;
        uint64_t    transactionSmsCount,  maxTransactionSms;
        time_t      transactionStartTime, maxTransactionTime;
        Hash<File::offset_type> transactionTrsArcFiles;
        Hash<File::offset_type> transactionTrsTxtFiles;
        Hash<bool>              transactionSrcFiles;

        void cleanTransaction();
        void startTransaction();
        void commitTransaction(bool force=false);
        void rollbackTransaction();

        void skipProcessedFiles(const std::string& location, Array<std::string>& files);
        bool process(const std::string& location, const Array<std::string>& files);

    public:

        ArchiveProcessor(ConfigView* config);
        virtual ~ArchiveProcessor();

        void init(ConfigView* config);
        void process();

        virtual int Execute(); // for socket accept && process queries on thread pool
        using Thread::Start;
        void Start();
        void Stop();

        inline SmsIndex* getIndexator() {
            return indexator;
        }
        inline const std::string& getBaseDirectory() {
            MutexGuard guard(directoriesLock);
            return baseDirectory;
        }
        inline const std::string& getTextDirectory() {
            MutexGuard guard(directoriesLock);
            return textDirectory;
        }
    };

}}

#endif

