
#define _FILE_OFFSET_BITS 64  // Win32
#define __USE_FILE_OFFSET64   // X     TODO: Move it to makefile

#include "ArchiveProcessor.h"
#include "FileStorage.h"

namespace smsc { namespace store
{

Mutex        Query::readLock;
EventMonitor Query::writeMonitor;
int          Query::activeCounter = 0;

/* -------------------- Archive Processor Implementation ------------------- */

ArchiveProcessor::ArchiveProcessor(ConfigView* config)
    : Thread(), log(Logger::getInstance("smsc.store.ArchiveProcessor")),
        bStarted(false), bNeedExit(false), indexator(0), bTransactionOpen(false), 
            transactionSmsCount(0),  maxTransactionSms(0), transactionStartTime(0), maxTransactionTime(0)
{
    init(config);
    Start();
}
ArchiveProcessor::~ArchiveProcessor()
{
    Stop();
    if (indexator) delete indexator;
}

void ArchiveProcessor::init(ConfigView* config)
{
    smsc_log_info(log, "Initing ...");

    std::auto_ptr<ConfigView> queriesThreadPoolCfgGuard(config->getSubConfig("Queries"));
    ConfigView* queriesThreadPoolCfg = queriesThreadPoolCfgGuard.get();
    try { queriesPool.setMaxThreads(queriesThreadPoolCfg->getInt("max")); }
    catch (ConfigException& exc) {
        smsc_log_warn(log, "Maximum threads count for queries wasn't specified !");
    }
    try { queriesPool.preCreateThreads(queriesThreadPoolCfg->getInt("init")); }
    catch (ConfigException& exc) {
        smsc_log_warn(log, "Precreated threads count for queries wasn't specified !");
    }

    std::auto_ptr<ConfigView> transactionCfgGuard(config->getSubConfig("Transactions"));
    ConfigView* transactionCfg = transactionCfgGuard.get();
    try { maxTransactionSms  = transactionCfg->getInt("maxSmsCount"); }
    catch (ConfigException& exc) {
        maxTransactionSms = 10000;
        smsc_log_warn(log, "Maximum sms per transaction parameter missed! Using default=%lld. Details: %s",
                      maxTransactionSms, exc.what());
    }
    try { maxTransactionTime = transactionCfg->getInt("maxTimeInterval"); }
    catch (ConfigException& exc) {
        maxTransactionTime = 300;
        smsc_log_warn(log, "Maximum transaction time interval parameter missed! Using default=%ld (sec). Details: %s",
                      maxTransactionTime, exc.what());
    }

    std::auto_ptr<ConfigView> viewCfgGuard(config->getSubConfig("View"));
    ConfigView* viewCfg = viewCfgGuard.get();
    const char* host = viewCfg->getString("host");
    int port = viewCfg->getInt("port");
    int timeout = viewCfg->getInt("timeout");
    if (serverSocket.InitServer(host, port, timeout) != 0)
        throw ConfigException("Failed to init server socket %s:%u", host, port);

    smsc_log_info(log, "Loading locations ...");

    MutexGuard guard(locationsLock);
    std::auto_ptr<ConfigView> locCfgGuard(config->getSubConfig("Locations"));
    ConfigView* locCfg = locCfgGuard.get();
    baseDirectory = locCfg->getString("baseDestination");
    textDirectory = locCfg->getString("textDestination");
    std::auto_ptr<ConfigView> sourcesCfgGuard(locCfg->getSubConfig("sources"));
    ConfigView* sourcesCfg = sourcesCfgGuard.get();

    std::auto_ptr< std::set<std::string> > setGuard(sourcesCfg->getStrParamNames());
    std::set<std::string>* set = setGuard.get();
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        try
        {
            const char* locId = (const char *)i->c_str();
            if (!locId || locId[0] == '\0')
                throw ConfigException("Source location id is empty or wasn't specified");

            const char* locDir = sourcesCfg->getString(locId);
            if (!locDir || locDir[0] == '\0')
                throw ConfigException("Source location value '%s' empty or wasn't specified", locId);

            std::string location = locDir;
            locations.Insert(locId, location);
        }
        catch (ConfigException& exc) {
            smsc_log_error(log, "Load of source locations failed ! Config exception: %s", exc.what());
            throw;
        }
    }

    smsc_log_info(log, "Initing indexator system ...");

    std::auto_ptr<ConfigView> indexatorCfgGuard(config->getSubConfig("Indexator"));
    indexator = new SmsIndex(baseDirectory.c_str());
    indexator->Init(indexatorCfgGuard.get());

    smsc_log_info(log, "Init Ok.");
}
void ArchiveProcessor::Start()
{
    MutexGuard guard(startLock);

    if (!bStarted)
    {
        smsc_log_info(log, "Starting ...");
        bNeedExit = false;
        bStarted = true;
        Thread::Start();
        smsc_log_info(log, "Started.");
    }
}
void ArchiveProcessor::Stop()
{
    MutexGuard  guard(startLock);

    if (bStarted)
    {
        smsc_log_info(log, "Stopping ...");
        bNeedExit = true;
        serverSocket.Abort();
        exited.Wait();
        bStarted = false;
        smsc_log_info(log, "Stoped.");
    }
}
int ArchiveProcessor::Execute()
{
    serverSocket.StartServer();
    while (!bNeedExit)
    {
        // Accept server socket here & process queries on ThreadPool
        //smsc_log_debug(log, "Waiting query socket ...");
        if (Socket *newSocket = serverSocket.Accept()) {
            smsc_log_info(log, "Query socket accepted.");
            queriesPool.startTask(new Query(this, newSocket));
        }
    }
    queriesPool.shutdown();
    exited.Signal();
    return 0;
}

void ArchiveProcessor::cleanTransaction()
{
    transactionTrsFiles.Empty();
    transactionSrcFiles.Empty();
    transactionSmsCount = 0;
}
void ArchiveProcessor::startTransaction()
{
    if (bTransactionOpen) return;
    
    try
    {
        smsc_log_debug(log, "Starting transaction...");
        if (indexator) indexator->BeginTransaction();
        bTransactionOpen = true; cleanTransaction();
        transactionStartTime = time(NULL);
        smsc_log_debug(log, "Transaction started.");

    } catch (std::exception& exc) {
      smsc_log_error(log, "Error starting transaction. Details: %s", exc.what());
    }
}
void ArchiveProcessor::commitTransaction(bool force /*= false*/)
{
    if (!bTransactionOpen) return;

    try
    {
        if (force || (transactionSmsCount >= maxTransactionSms) ||
            (transactionStartTime>0 && (time(NULL)-transactionStartTime >= maxTransactionTime)))
        {
            smsc_log_debug(log, "Commiting transaction...");
            if (indexator) indexator->EndTransaction();

            // flush transaction files
            transactionTrsFiles.First();
            char* trsFileNameStr = 0; fpos_t* trsPosition = 0;
            while (transactionTrsFiles.Next(trsFileNameStr, trsPosition))
            {
                if (!trsPosition || !trsFileNameStr || !trsFileNameStr[0]) continue;
                TransactionStorage trsFile(trsFileNameStr);
                trsFile.setTransactionData(trsPosition);
            }
            // delete source files
            transactionSrcFiles.First();
            char* srcFileNameStr = 0; bool data = false;
            while (transactionSrcFiles.Next(srcFileNameStr, data))
            {
                if (!srcFileNameStr || !srcFileNameStr[0]) continue;
                FileStorage::deleteFile(srcFileNameStr);
            }
            bTransactionOpen = false; cleanTransaction();
            smsc_log_debug(log, "Transaction commited.");
        }
    
    } catch (std::exception& exc) {
      smsc_log_error(log, "Error commiting transaction. Details: %s", exc.what());
    }
    
}
void ArchiveProcessor::rollbackTransaction()
{
    if (!bTransactionOpen) return;
    
    smsc_log_debug(log, "Rolling back transaction...");
    if (indexator) indexator->RollBack();
    bTransactionOpen = false; cleanTransaction();
    smsc_log_debug(log, "Transaction rolled back.");
}
void ArchiveProcessor::skipProcessedFiles(const std::string& location, Array<std::string>& files)
{
    for (int i=0; i<files.Count(); i++) {
        std::string fileName = location+"/"+files[i];
        const char* fileNameStr = fileName.c_str();
        if (transactionSrcFiles.Exists(fileNameStr)) files.Delete(i--);
    }
}
void ArchiveProcessor::process()
{
    MutexGuard guard(locationsLock);

    smsc_log_debug(log, "Processing ...");
    commitTransaction(false);
    char* locId = 0; std::string* location = 0;
    locations.First();
    while (locations.Next(locId, location) && !bNeedExit)
    {
        if (!location) continue;
        smsc_log_debug(log, "Processing location '%s' ...", location->c_str());

        try
        {
            bool bProcessSuccess = false;
            while (!bProcessSuccess)
            {
                Array<std::string> files;
                FileStorage::findFiles(*location, SMSC_PREV_ARCHIVE_FILE_EXTENSION, files);
                skipProcessedFiles(*location, files); // filter out already processed files
                if (files.Count() > 0) {
                    startTransaction();
                    bProcessSuccess = process(*location, files);
                    if (!bProcessSuccess) rollbackTransaction();
                } else {
                    smsc_log_debug(log, "No new archive files found in '%s'", location->c_str());
                    break;
                }
            }

        } catch (std::exception& exc) {
          smsc_log_error(log, "Error processing archive files. Details: %s", exc.what());
        } catch (...) {
          smsc_log_error(log, "Error processing archive files. Reason is unknown");
        }
    }
    commitTransaction(bNeedExit);
    smsc_log_debug(log, "Processed.");
}

static bool switchDate(time_t date1, time_t date2, char* destinationFile,
                       char* destinationDir=0, bool addExt=true)
{
    tm dt1, dt2; gmtime_r(&date1, &dt1); gmtime_r(&date2, &dt2);

    bool result = false;
    if (date1 <= 0 || dt1.tm_mday != dt2.tm_mday || dt1.tm_mon != dt2.tm_mon || dt1.tm_year != dt2.tm_year) {
        if (destinationDir) {
            sprintf(destinationDir, SMSC_PERSIST_DIR_NAME_PATTERN,
                    dt2.tm_year+1900, dt2.tm_mon+1, dt2.tm_mday);
        }
        result = true;
    }
    if (date1 <= 0 || dt1.tm_hour != dt2.tm_hour) {
        sprintf(destinationFile, "%02d%s%s", dt2.tm_hour,
                (addExt) ? ".":"", (addExt) ? SMSC_PREV_ARCHIVE_FILE_EXTENSION:"");
        result = true;
    }
    return result;
}

bool ArchiveProcessor::process(const std::string& location, const Array<std::string>& files)
{
    Query::ProcessArchiveGuard archiveGuard;

    char destinationFileName[64];
    char destinationDirName[64];
    time_t lastProcessedTime = 0;

    for (int i=0; i<files.Count() && !bNeedExit; i++)
    {
        std::string file = files[i];
        smsc_log_debug(log, "Processing archive file '%s' ...", file.c_str());

        std::string srcFileName = location+"/"+file;
        const char* srcFileNameStr = srcFileName.c_str();
        if (!srcFileNameStr || !srcFileNameStr[0]) continue;
        
        PersistentStorage*  arcDestination = 0;
        TextDumpStorage*    txtDestination = 0;
        
        try
        {
            hrtime_t prtime=gethrtime();
            long long count=0;
            {
                std::string trsFileName = "";
                
                PersistentStorage source(location, file);
                startTransaction(); // start transactional indecies
                if (!transactionSrcFiles.Exists(srcFileNameStr))
                     transactionSrcFiles.Insert(srcFileNameStr, true);

                while (true)
                {
                    SMSId id; SMS sms;
                    if (!source.readRecord(id, sms, 0)) break;
                    bool bNewArcFile = false; count++;

                    if (switchDate(lastProcessedTime, sms.lastTime, destinationFileName, destinationDirName, false)
                        || !arcDestination || !txtDestination)
                    {
                        if (arcDestination) { delete arcDestination; arcDestination=0; }
                        if (txtDestination) { delete txtDestination; txtDestination=0; }

                        std::string arcLocation = baseDirectory+'/'+destinationDirName;
                        std::string txtLocation = textDirectory+'/'+destinationDirName;
                                    trsFileName = arcLocation+'/'+destinationFileName; 
                        std::string arcFileName = destinationFileName;
                        std::string txtFileName = destinationFileName;
                        arcFileName += '.'; arcFileName += SMSC_PREV_ARCHIVE_FILE_EXTENSION;
                        txtFileName += '.'; txtFileName += SMSC_TEXT_ARCHIVE_FILE_EXTENSION;
                        trsFileName += '.'; trsFileName += SMSC_TRNS_ARCHIVE_FILE_EXTENSION;

                        FileStorage::createDir(arcLocation);
                        FileStorage::createDir(txtLocation);

                        arcDestination = new PersistentStorage(arcLocation, arcFileName);
                        txtDestination = new TextDumpStorage(txtLocation, txtFileName);

                        lastProcessedTime = sms.lastTime; bNewArcFile = true;
                        smsc_log_debug(log, "Opened destination file '%s/%s'", destinationDirName, arcFileName.c_str());
                    }

                    try
                    { 
                        fpos_t position = 0;
                        arcDestination->openWrite(&position);
                        //smsc_log_debug(log, "Archive file position=%lld", position);
                        
                        const char* trsFileNameStr = trsFileName.c_str();
                        fpos_t* trsPosition = 0;
                        if (bNewArcFile) // destination file was switched
                        {
                            bNewArcFile = false;
                            trsPosition = transactionTrsFiles.GetPtr(trsFileNameStr);
                            if (!trsPosition) // check reopened destination file
                            {
                                fpos_t trans_position = 0;
                                TransactionStorage trsFile(trsFileName);
                                if (trsFile.getTransactionData(&trans_position))
                                {
                                    if (trans_position < 0 || position < trans_position)
                                        throw Exception("Invalid transaction position=%lld, file end=%lld",
                                                        trans_position, position);
                                    position = trans_position;
                                    arcDestination->setPos(&position);
                                }
                                else trsFile.setTransactionData(&position);
                            }
                            else {
                                smsc_log_debug(log, "File '%s' was re-opened, last position=%lld, file end=%lld",
                                               file.c_str(), *trsPosition, position);
                                position = *trsPosition;
                                arcDestination->setPos(&position);
                            }
                        }
                        
                        indexator->IndexateSms(destinationDirName, id, (uint64_t)position, sms);

                        txtDestination->writeRecord(id, sms);
                        arcDestination->writeRecord(id, sms);
                        arcDestination->getPos(&position);
                        transactionSmsCount++;
                        
                        trsPosition = transactionTrsFiles.GetPtr(trsFileNameStr);
                        if (trsPosition) *trsPosition = position;
                        else transactionTrsFiles.Insert(trsFileNameStr, position);
                    }
                    catch (DiskHashDuplicateKeyException& duplicateExc) {
                        smsc_log_warn(log, "SMS #%lld in file '%s' skipped. Details: %s.",
                                      id, file.c_str(), duplicateExc.what());
                    }
                } // while all records from source file will not be fetched

                if (arcDestination) { delete arcDestination; arcDestination=0; }
                if (txtDestination) { delete txtDestination; txtDestination=0; }
            }
            prtime=gethrtime()-prtime;
            double tmInSec=(double)prtime/1000000000.0L;
            smsc_log_info(log,"Processed file '%s' in %lld msec, %lld sms found (%lld in transaction), "
                          "processing speed %lf sms/sec", file.c_str(), prtime/1000000, 
                          count, transactionSmsCount, (double)count/tmInSec);
            
            commitTransaction(false); // flush transactional indecies

        } catch (std::exception& exc) {
          smsc_log_error(log, "Error processing archive file '%s'. Details: %s", file.c_str(), exc.what());
          try {
              smsc_log_info(log, "Rolling '%s' to '*.err' ...", file.c_str());
              FileStorage::rollErrorFile(location, file);
          } catch (...) { smsc_log_error(log, "Failed to rool error file '%s'", file.c_str()); }
          if (arcDestination) { delete arcDestination; arcDestination=0; }
          if (txtDestination) { delete txtDestination; txtDestination=0; }
          return false;
        } catch (...) {
          smsc_log_error(log, "Error processing archive file '%s'. Reason is unknown", file.c_str());
          try {
              smsc_log_info(log, "Rolling '%s' to '*.err' ...", file.c_str());
              FileStorage::rollErrorFile(location, file);
          } catch (...) { smsc_log_error(log, "Failed to rool error file '%s'", file.c_str()); }
          if (arcDestination) { delete arcDestination; arcDestination=0; }
          if (txtDestination) { delete txtDestination; txtDestination=0; }
          return false;
        }
    }
    return true;
}

/* -------------------------- Query Implementation ------------------------- */

Query::ProcessArchiveGuard::ProcessArchiveGuard()
{
    Query::readLock.Lock();

    MutexGuard wlg(Query::writeMonitor);
    while(Query::activeCounter > 0) {
        Query::writeMonitor.wait();
    }
}
Query::ProcessArchiveGuard::~ProcessArchiveGuard()
{
    Query::readLock.Unlock();
}
Query::ProcessQueryGuard::ProcessQueryGuard()
{
    MutexGuard rlg(Query::readLock);
    Query::activeCounter++;
}
Query::ProcessQueryGuard::~ProcessQueryGuard()
{
    MutexGuard wlg(Query::writeMonitor);
    Query::activeCounter--;
    Query::writeMonitor.notifyAll();
}

Query::Query(ArchiveProcessor* processor, Socket *socket)
    : ThreadedTask(), log(Logger::getInstance("smsc.store.ArchiveQuery")),
        processor(processor), socket(socket), messagesToSend(0)
{
}
Query::~Query()
{
    if (socket) delete socket;
}

void Query::findDirsByQuery(QueryMessage* query, const std::string& location,
                            Array<DirEntry>& dirs)
{
    Array<std::string> allDirs;
    FileStorage::findDirs(location, allDirs);

    tm tmdt; int64_t fd = -1; int64_t td = -1;
    if (query->fromDate > 0) {
        gmtime_r(&(query->fromDate), &tmdt);
        fd = (tmdt.tm_year+1900)*10000+(tmdt.tm_mon+1)*100+tmdt.tm_mday;
    }
    if (query->tillDate > 0) {
        gmtime_r(&(query->tillDate), &tmdt);
        td = (tmdt.tm_year+1900)*10000+(tmdt.tm_mon+1)*100+tmdt.tm_mday;
    }
    for (int i=0; i<allDirs.Count(); i++)
    {
        std::string dir = allDirs[i];
        if (sscanf(dir.c_str(), "%04d%02d%02d", &tmdt.tm_year, &tmdt.tm_mon, &tmdt.tm_mday) != 3) {
            smsc_log_warn(log, "Invalid archive directory name format '%s'", dir.c_str());
            continue;
        }
        int64_t dd = tmdt.tm_year*10000+tmdt.tm_mon*100+tmdt.tm_mday;
        if (fd > 0 && dd < fd) continue;
        if (td > 0 && dd > td) continue;

        dirs.Push(DirEntry(dir, dd));
    }
}
void Query::findFilesByQuery(QueryMessage* query, const std::string& location,
                             uint64_t dirCode, Array<uint8_t>& files)
{
    Array<std::string> allFiles;
    FileStorage::findFiles(location, SMSC_PREV_ARCHIVE_FILE_EXTENSION, allFiles);

    tm tmdt; int64_t fd = -1; int64_t td = -1;
    if (query->fromDate > 0) {
        gmtime_r(&(query->fromDate), &tmdt);
        fd = (tmdt.tm_year+1900)*1000000+(tmdt.tm_mon+1)*10000+tmdt.tm_mday*100+tmdt.tm_hour;
    }
    if (query->tillDate > 0) {
        gmtime_r(&(query->tillDate), &tmdt);
        td = (tmdt.tm_year+1900)*1000000+(tmdt.tm_mon+1)*10000+tmdt.tm_mday*100+tmdt.tm_hour;
    }

    for (int i=0; i<allFiles.Count(); i++)
    {
        std::string file = allFiles[i];
        uint8_t hour;
        if (sscanf(file.c_str(), "%02d.arc", &hour) != 1) {
            smsc_log_warn(log, "Invalid archive file name format '%s'", file.c_str());
            continue;
        }
        uint64_t dd = dirCode*100+hour;
        if (fd > 0 && dd < fd) continue;
        if (td > 0 && dd > td) continue;

        files.Push(hour);
    }
}

bool Query::prepareIndex(QueryMessage* query, Array<Param>& index)
{
    index.Clean(); bool noIdIndex = true;
    for (int i=0; i<query->parameters.Count() && noIdIndex; i++)
    {
        Param param = query->parameters[i];
        switch(param.type)
        {
        case T_SMS_ID:
            // Exclusive index by ID, other checks will be performed in checkMessage()
            index.Clean(); index.Push(param); noIdIndex = false;
            break;
        case T_FROM_DATE: case T_TILL_DATE:
            break;
        case T_SRC_ADDRESS: case T_DST_ADDRESS: case T_ABN_ADDRESS: {
            // Skip masks for search index. All checks will be performed in checkMessage()
            const char* str = param.sValue.c_str();
            if (!str || str[0] == '\0') break;
            if (!strchr(str, '?')) index.Push(param);
            else {
                if (param.type == T_SRC_ADDRESS) { query->bSrcMask = true; query->srcMask = param.sValue; }
                if (param.type == T_DST_ADDRESS) { query->bDstMask = true; query->dstMask = param.sValue; }
                if (param.type == T_ABN_ADDRESS) { query->bAbnMask = true; query->abnMask = param.sValue; }
            }
            break;
        }
        case T_SME_ID: case T_SRC_SME_ID: case T_DST_SME_ID: case T_ROUTE_ID:
            index.Push(param);
            break;
        default:
            throw CommunicationException("Invalid query parameter type %u", param.type);
        }
    }
    bool needIndex = (index.Count() > 0);
    if (query->fromDate > 0) {
        Param param; param.dValue = query->fromDate;
        param.type = Param::tFromDate; index.Push(param);
    }
    if (query->tillDate > 0) {
        Param param; param.dValue = query->tillDate;
        param.type = Param::tTillDate; index.Push(param);
    }
    return needIndex;
}

bool eqAdressWithMask(const Address& address, const std::string& mask)
{
    char aBuf[64];
    int al = address.toString(aBuf, sizeof(aBuf));
    if (al != mask.length()) return false;
    const char* aStr = aBuf; const char* mStr = mask.c_str();

    for (;*aStr && *mStr; aStr++, mStr++)
        if (*aStr != *mStr && *mStr != '?') return false;

    return (!*aStr && !*mStr);
}
bool Query::checkMessage(QueryMessage* query, SMSId id, SMS& sms)
{
    if (query->fromDate > 0 && sms.lastTime < query->fromDate) return false;
    if (query->tillDate > 0 && sms.lastTime > query->tillDate) return false;

    if (query->bSrcMask && !eqAdressWithMask(sms.getOriginatingAddress(),          query->srcMask)) return false;
    if (query->bDstMask && !eqAdressWithMask(sms.getDealiasedDestinationAddress(), query->dstMask)) return false;
    if (query->bAbnMask && !eqAdressWithMask(sms.getOriginatingAddress(),          query->abnMask)
                        && !eqAdressWithMask(sms.getDestinationAddress(),          query->abnMask)
                        && !eqAdressWithMask(sms.getDealiasedDestinationAddress(), query->abnMask)) return false;

    return true;
}
bool Query::sendMessage(DaemonCommunicator& communicator, SMSId id, SMS& sms)
{
    if (messagesToSend <= 0)
    {
        std::auto_ptr<Message> messageGuard(communicator.receive());
        Message* message = messageGuard.get();
        if (!message)
            throw Exception("Received message is NULL!");

        switch (message->type)
        {
        case Message::RSNXT:
            messagesToSend = ((RsNxtMessage *)message)->next;
            //printf("To send=%d\n", messagesToSend);
            break;
        case Message::EMPTY:
            return false;
        case Message::ERROR:
            throw CommunicationException("Error message received: '%s'",
                                         ((ErrorMessage *)message)->error.c_str());
        default:
            throw CommunicationException("Unexpected message received type is %u", message->type);
        }
    }

    RsSmsMessage responce(id, sms);
    communicator.send(&responce);
    messagesToSend--;
    return true;
}

static void checkDateForDir(uint64_t dircode, time_t date)
{
    tm tmdt; gmtime_r(&date, &tmdt);
    uint64_t td = (tmdt.tm_year+1900)*10000+(tmdt.tm_mon+1)*100+tmdt.tm_mday;
    if (date <= 0 || dircode != td)
        throw Exception("Date %lld (%d) is invalid for dircode %lld", td, date, dircode);
}

int Query::Execute()
{
    __require__(processor);

    smsc_log_info(log, "Query processing ...");

    Query::ProcessQueryGuard queryGuard;
    DaemonCommunicator communicator(socket);
    QueryMessage* query = 0;

    SmsIndex* indexator = processor->getIndexator();
    std::string baseDir = processor->getBaseDirectory();

    time_t lastProcessedTime = 0;
    PersistentStorage* source = 0;
    char sourceFileName[64]; sourceFileName[0]=0;

    try
    {
        uint64_t totalMessages = 0; messagesToSend = 0;
        Message* message = communicator.receive();
        if (!message)
            throw CommunicationException("Received message is NULL!");
        if (message->type != Message::QUERY && message->type != Message::COUNT)
            throw CommunicationException("Expected QUERY or COUNT message instead of %u type", message->type);
        query = (QueryMessage *)message;

        Array<Param>   index; // trimmed parameters for indexator
        bool needIndex = prepareIndex(query, index);
        bool moreMessagesToSend = true;

        smsc_log_info(log, "Searching %s ...", needIndex ? "by index":"via full scan");

        Array<DirEntry> scanDirs;
        findDirsByQuery(query, baseDir, scanDirs);

        for (int i=0; i<scanDirs.Count() && moreMessagesToSend; i++)
        {
            DirEntry dir = scanDirs[i];
            std::string location = baseDir; location+='/'; location+=dir.dir;
            smsc_log_info(log, "Scanning dir: %s", location.c_str());
            if (needIndex)
            {
                if (!indexator) throw Exception("Indexator is not avaliable !");

                ResultArray indecies;
                indexator->QuerySms(dir.dir.c_str(), index, indecies);

                for (int j=0; j<indecies.Count() && moreMessagesToSend; j++)
                {
                    QueryResult& idx = indecies[j];

                    /*smsc_log_info(log, "Index offset: %lld, time: %ld %s", idx.offset,
                                  idx.lastTryTime, ctime((time_t *)&idx.lastTryTime));*/
                    checkDateForDir(dir.code, idx.lastTryTime);

                    if (switchDate(lastProcessedTime, (time_t)idx.lastTryTime, sourceFileName) || !source)
                    {
                        smsc_log_info(log, "Scanning file: %s", sourceFileName);
                        if (source) delete source;
                        source = new PersistentStorage(location, sourceFileName);
                        lastProcessedTime = idx.lastTryTime;
                    }

                    SMSId id = 0; SMS sms;
                    fpos_t position = idx.offset;
                    if (!source->readRecord(id, sms, &position)) break;
                    if (!checkMessage(query, id, sms)) continue;
                    totalMessages++;
                    if (query->type == Message::COUNT) continue;
                    if (!sendMessage(communicator, id, sms)) {
                        moreMessagesToSend = false;
                        break;
                    }
                    if (!(totalMessages%100))
                        smsc_log_info(log, "%lld messages sent", totalMessages);
                }
            }
            else
            {
                Array<uint8_t> scanFiles;
                findFilesByQuery(query, location, dir.code, scanFiles);
                char arcFileName[64]; char trsFileName[64];
                for (int j=0; j<scanFiles.Count() && moreMessagesToSend; j++)
                {
                    sprintf(arcFileName, "%02d.%s", scanFiles[j], SMSC_PREV_ARCHIVE_FILE_EXTENSION);
                    sprintf(trsFileName, "%02d.%s", scanFiles[j], SMSC_TRNS_ARCHIVE_FILE_EXTENSION);

                    smsc_log_info(log, "Scanning file: %s", arcFileName);
                    PersistentStorage  arcFile(location, arcFileName);
                    TransactionStorage trsFile(location, trsFileName);
                    bool isTrans = false; fpos_t position = 0; fpos_t trans_position = 0; 
                    isTrans = trsFile.getTransactionData(&trans_position);
                    arcFile.openRead(0);
                    while (moreMessagesToSend)
                    {
                        SMSId id = 0; SMS sms;
                        if (isTrans) {
                            arcFile.getPos(&position);
                            if (position > trans_position) {
                                smsc_log_warn(log, "EOF reached in file '%s' (cur_pos=%lld, trans_pos=%lld)",
                                              arcFileName, position, trans_position);
                                break;
                            }
                        }
                        if (!arcFile.readRecord(id, sms)) break;
                        if (!checkMessage(query, id, sms)) continue;
                        totalMessages++;
                        if (query->type == Message::COUNT) continue;
                        if (!sendMessage(communicator, id, sms)) {
                            moreMessagesToSend = false;
                            break;
                        }
                        if (!(totalMessages%100))
                            smsc_log_info(log, "%lld messages sent", totalMessages);
                    }
                }
            }
        }

        if (query->type != Message::COUNT) {
            EmptyMessage empty;
            communicator.send(&empty);
        } else {
            TotalMessage total(totalMessages);
            communicator.send(&total);
        }
    }
    catch (EOFException& exc) {
        smsc_log_error(log, "User query cancelled. Details: %s", exc.what());
    }
    catch (std::exception& exc) {
        smsc_log_error(log, "Error processing user query. Details: %s", exc.what());
        try { ErrorMessage error(exc.what()); communicator.send(&error); }
        catch (...) { smsc_log_error(log, "Failed to send error message."); }
    }
    catch (...) {
        smsc_log_error(log, "Error processing user query. Reason is unknown");
        try { ErrorMessage error("Unknown error"); communicator.send(&error); }
        catch (...) { smsc_log_error(log, "Failed to send error message."); }
    }

    if (source) delete source;
    if (query) delete query;
    smsc_log_info(log, "Query processed.");

    return 0;
}


}}
