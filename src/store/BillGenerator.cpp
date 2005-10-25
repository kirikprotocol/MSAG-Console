
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include <sms/sms.h>
#include <util/xml/init.h>

#include <smpp/smpp_structures.h>
#include <util/csv/CSVFileEncoder.h>
#include <util/recoder/recode_dll.h>
#include <util/smstext.h>

#include "FileStorage.h"

using namespace smsc::sms;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::store;

using namespace smsc::util::csv;
using smsc::smpp::DataCoding;

static smsc::logger::Logger* logger = 0;

static std::string archiveLocation;
static std::string billingLocation;

class BillingDumper : public FileStorage
{
protected:

    time_t lastTime;
    char   storageFileName[256];
    
    virtual void initialize(bool flag) {};

    void create(time_t nextTime)
    {
        tm ldt, ndt; gmtime_r(&lastTime, &ldt); gmtime_r(&nextTime, &ndt);
        if (lastTime <= 0 || ldt.tm_hour != ndt.tm_hour || ldt.tm_mday != ndt.tm_mday || 
                             ldt.tm_mon  != ndt.tm_mon  || ldt.tm_year != ndt.tm_year) 
        {
            lastTime = nextTime;
            try { if (storageFile.isOpened()) storageFile.Close(); }
            catch (FileException& fexc) {
                Exception exc("Failed to close generated billing file. Details: %s", fexc.what());
                throw StorageException(exc.what());
            }
        }
        if (!storageFile.isOpened())
        {
            sprintf(storageFileName, "%04d%02d%02d_%02d%02d%02d.csv",
                    ndt.tm_year+1900, ndt.tm_mon+1, ndt.tm_mday, ndt.tm_hour, 0, 0);
            std::string fullFilePath = storageLocation;
            fullFilePath += '/'; fullFilePath += (const char*)storageFileName;
            const char* fullFilePathStr = fullFilePath.c_str();

            bool fileFound = false;
            try 
            {
                fileFound = File::Exists(fullFilePathStr);
                if (fileFound) {
                    storageFile.WOpen(fullFilePathStr);
                    storageFile.SeekEnd(0);
                }
                else storageFile.RWCreate(fullFilePathStr);
            }
            catch (FileException& fexc) {
                Exception exc("Failed to create new billing file '%s'. Details: %s",
                              fullFilePathStr, fexc.what());
                try { if (storageFile.isOpened()) storageFile.Close(); } catch(...) {}
                throw StorageException(exc.what());
            }

            if (fileFound) {
                static const char* SMSC_BILLING_HEADER_TEXT =
                    "MSG_ID,RECORD_TYPE,MEDIA_TYPE,BEARER_TYPE,SUBMIT,FINALIZED,STATUS,"
                    "SRC_ADDR,SRC_IMSI,SRC_MSC,SRC_SME_ID,DST_ADDR,DST_IMSI,DST_MSC,DST_SME_ID,"
                    "DIVERTED_FOR,ROUTE_ID,SERVICE_ID,USER_MSG_REF,DATA_LENGTH\n";
                FileStorage::write( SMSC_BILLING_HEADER_TEXT, strlen(SMSC_BILLING_HEADER_TEXT));
                FileStorage::flush();
            }
        }
    };

public:

    BillingDumper(const std::string& location) : FileStorage() { 
        storageLocation = location; lastTime = -1;
    };
    virtual ~BillingDumper() {};

    void createRecord(SMSId id, SMS& sms)
    {
        std::string out; out.reserve(512);
        FileStorage::bill(id, sms, out);
        {
            MutexGuard guard(storageFileLock);
            this->create(sms.lastTime);
            FileStorage::write(out.c_str(), out.length());
            FileStorage::flush();
        }
    }

};

static time_t parseDateTime(const char* str)
{
    int year, month, day, hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d.%02d.%4d %02d:%02d:%02d", 
                    &day, &month, &year, &hour, &minute, &second) != 6) return -1;
    
    tm  dt; dt.tm_isdst = -1;
    dt.tm_year = year - 1900; dt.tm_mon = month - 1; dt.tm_mday = day;
    dt.tm_hour = hour; dt.tm_min = minute; dt.tm_sec = second;
    return mktime(&dt);
}


static Hash<bool> routesFilter;
static time_t     fromDateFilter = -1;
static time_t     tillDateFilter = -1;

bool checkFilter(SMSId id, const SMS& sms)
{
    if (fromDateFilter > 0 && sms.lastTime <  fromDateFilter) return false;
    if (tillDateFilter > 0 && sms.lastTime >= tillDateFilter) return false;
    return routesFilter.Exists(sms.routeId);
}

void printUsage()
{
    printf("Billing records generator v1.0 Usage:\nBillGenerator "
           "[-f fromDate] [-t tillDate] route_id_1[...route_id_N]\nParameters:\n"
           "\tfromDate  :\tdate/time to start extract billing records\n"
           "\ttillDate  :\tdate/time to stop  extract billing records\n"
           "\tdateFormat:\tdd.MM.yyyy hh:mm:ss\n"
           "\troute_id_i:\troute id to check (at least one must be specified)\n");
}
bool parseParams(int argc, char* argv[])
{
    bool paramsParsed = false;
    for (int i=1; i<argc; i++)
    {
        char* param = argv[i];
        if (!param || !param[0]) continue;
        //printf("P%d: %s\n", i, param);

        if (param[0] == '-' && (i+1 < argc)) {
            if (param[1] == 'f') {
                fromDateFilter = parseDateTime(argv[++i]);
                if (fromDateFilter <= 0) { 
                    printf("Invalid from date format '%s'\n", argv[i]);
                    break;
                }
                //else printf("%s", ctime(&tillDateFilter));
            }
            else if (param[1] == 't') {
                tillDateFilter = parseDateTime(argv[++i]);
                if (tillDateFilter <= 0) {
                    printf("Invalid till date format '%s'\n", argv[i]);
                    break;
                }
                //else printf("%s", ctime(&tillDateFilter));
            }
            else { 
                printf("Invalid option '%s'\n", param);
                break;
            }
        }
        else {
            if (!routesFilter.Exists(param)) routesFilter.Insert(param, true);
            paramsParsed = true;
        }
    }
    return paramsParsed;
}
void findDirs(const std::string& location, Array<std::string>& dirs)
{
    Array<std::string> allDirs;
    FileStorage::findDirs(location, allDirs);

    tm tmdt; int64_t fd = -1; int64_t td = -1;
    if (fromDateFilter > 0) {
        gmtime_r(&fromDateFilter, &tmdt);
        fd = (tmdt.tm_year+1900)*10000+(tmdt.tm_mon+1)*100+tmdt.tm_mday;
    }
    if (tillDateFilter > 0) {
        gmtime_r(&tillDateFilter, &tmdt);
        td = (tmdt.tm_year+1900)*10000+(tmdt.tm_mon+1)*100+tmdt.tm_mday;
    }
    for (int i=0; i<allDirs.Count(); i++)
    {
        std::string dir = allDirs[i];
        if (sscanf(dir.c_str(), "%04d%02d%02d", &tmdt.tm_year, &tmdt.tm_mon, &tmdt.tm_mday) != 3) {
            smsc_log_warn(logger, "Invalid archive directory name format '%s'", dir.c_str());
            continue;
        }
        int64_t dd = tmdt.tm_year*10000+tmdt.tm_mon*100+tmdt.tm_mday;
        if (fd > 0 && dd < fd) continue;
        if (td > 0 && dd > td) continue;
        
        dirs.Push(dir);
    }
}

int main(int argc, char* argv[])
{
    using smsc::util::config::Manager;
    using smsc::util::config::ConfigView;
    using smsc::util::config::ConfigException;
    
    if (!parseParams(argc, argv)) {
        printUsage();
        return -1;
    }
    
    Logger::Init();
    logger = Logger::getInstance("smsc.store.BillGenerator");
    int resultCode = 0;
    try 
    {
        Manager::init("generator.xml");
        ConfigView appConfig(Manager::getInstance(), "BillGenerator");
        std::auto_ptr<ConfigView> locCfgGuard(appConfig.getSubConfig("Locations"));
        ConfigView* locCfg = locCfgGuard.get();
        archiveLocation = locCfg->getString("archive");
        billingLocation = locCfg->getString("billing");

        smsc_log_info(logger, "Processing location: %s", archiveLocation.c_str());

        Array<std::string> dirs;
        findDirs(archiveLocation, dirs);
        if (dirs.Count() > 0)
        {
            BillingDumper billingDumper(billingLocation);
            uint64_t totalFilterredCount = 0;
            
            for (int j=0; j<dirs.Count(); j++)
            {
                std::string dir = dirs[j];
                std::string location = archiveLocation+'/'+dir;
                smsc_log_info(logger, "Processing archive dir: %s", dir.c_str());
                
                Array<std::string> files;
                FileStorage::findFiles(location, SMSC_PREV_ARCHIVE_FILE_EXTENSION, files);
                if (files.Count() > 0)
                {
                    for (int i=0; i<files.Count(); i++)
                    {
                        std::string file = files[i];
                        smsc_log_info(logger, "Processing file: %s/%s", dir.c_str(), file.c_str());

                        SMSId id; SMS sms;
                        PersistentStorage source(location, file);
                        hrtime_t prtime=gethrtime(); 
                        uint64_t recordsCount = 0; uint64_t filteredCount = 0;
                        while (source.readRecord(id, sms, 0))
                        {
                            if (!((++recordsCount)%10000)) smsc_log_debug(logger, "%lld records read", recordsCount);
                            if (!checkFilter(id, sms)) continue;
                            billingDumper.createRecord(id, sms);
                            filteredCount++;
                        }
                        prtime=gethrtime()-prtime;
                        double tmInSec=(double)prtime/1000000000.0L;
                        smsc_log_info(logger,"Processed file '%s/%s' in %lld msec, %lld sms found (%lld transferred), "
                                      "processing speed %lf sms/sec", dir.c_str(), file.c_str(), prtime/1000000, 
                                      recordsCount, filteredCount, (double)recordsCount/tmInSec);
                        totalFilterredCount += filteredCount;
                    }
                }
                else smsc_log_warn(logger, "No archive files found in '%s'", dir.c_str());
            }
            
            smsc_log_info(logger, "Processed Ok. %lld records created", totalFilterredCount);
        }
        else smsc_log_warn(logger, "No archive directories found in '%s'", archiveLocation.c_str());

    } catch (ConfigException& exc) {
        smsc_log_error(logger, "Configuration invalid. Details: %s Exiting.", exc.what());
        resultCode = -2;
    } catch (Exception& exc) {
        smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
        resultCode = -3;
    } catch (std::exception& exc) {
        smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
        resultCode = -4;
    } catch (...) {
        smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
        resultCode = -5;
    }

    return resultCode;
}

