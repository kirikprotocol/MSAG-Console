
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

#include <util/config/route/RouteConfig.h>
#include <util/config/smeman/SmeManConfig.h>
#include <router/route_manager.h>
#include <util/Timer.hpp>
#include <smeman/smeman.h>
#include <util/regexp/RegExp.hpp>
#include <alias/AliasManImpl.hpp>
#include <util/config/alias/aliasconf.h>

#include <util/findConfigFile.h>

#include <core/buffers/TmpBuf.hpp>
#include <core/buffers/XHash.hpp>
#include <core/buffers/Array.hpp>
#include <core/buffers/File.hpp>

using namespace smsc::sms;
using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::buffers;
using namespace smsc::router;
using namespace smsc::smeman;
using namespace smsc::alias;
using namespace smsc::util::config::alias;

static smsc::logger::Logger* logger = 0;

void reloadAliases(AliasManager* aliaser,const AliasConfig& cfg)
{
  {
    smsc::util::config::alias::AliasConfig::RecordIterator i =
                                cfg.getRecordIterator();
    while(i.hasRecord())
    {
      smsc::util::config::alias::AliasRecord *rec;
      i.fetchNext(rec);
      __trace2__("adding %20s %20s",rec->addrValue,rec->aliasValue);
      smsc::alias::AliasInfo ai;
      ai.addr = smsc::sms::Address(
        strlen(rec->addrValue),
        rec->addrTni,
        rec->addrNpi,
        rec->addrValue);
      ai.alias = smsc::sms::Address(
        strlen(rec->aliasValue),
        rec->aliasTni,
        rec->aliasNpi,
        rec->aliasValue);
      ai.hide = rec->hide;
      aliaser->addAlias(ai);
    }
    //aliaser->commit();
  }
}

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
static time_t convertTime(tm& dt)
{
    dt.tm_isdst = -1; dt.tm_mon--;
    return mktime(&dt);
}

static time_t fromDateFilter = -1;
static time_t tillDateFilter = -1;
static Array<std::string> logFiles;

void printUsage()
{
    printf("Stat records generator v1.0 Usage:\nStat4Log "
           "[-f fromDate] [-t tillDate] logfile1 [,logfileN ...]\n"
           "\tfromDate  :\tdate/time to start genarate stat records\n"
           "\ttillDate  :\tdate/time to stop  genarate stat records\n"
           "\tdateFormat:\tdd.MM.yyyy hh:mm:ss\n");
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
            if (!File::Exists(param)) {
                printf("Log file '%s' not found\n", param);
                continue;
            }
            logFiles.Push(param);
            paramsParsed = true;
        }
    }
    return paramsParsed;
}

namespace smsc { namespace system
{
    extern void loadRoutes(RouteManager* rm,
                           const smsc::util::config::route::RouteConfig& rc,
                           bool traceit=false);
}}

struct SmsStat
{
    uint32_t accepted, delivered;

    SmsStat(uint32_t a=0, uint32_t d=0) : accepted(a), delivered(d) {};
    SmsStat(const SmsStat& stat) : accepted(stat.accepted), delivered(stat.delivered) {};
    void Empty() { accepted = 0; delivered=0; };
    bool notEmpty() { return (accepted || delivered); };
};
struct RouteStat : public SmsStat
{
    signed long providerId, categoryId;

    RouteStat(uint32_t a=0, uint32_t d=0, signed long _providerId = -1, signed long _categoryId = -1)
        : SmsStat(), providerId(_providerId), categoryId(_categoryId) {};
    RouteStat(const RouteStat& stat)
        : SmsStat(stat), providerId(stat.providerId), categoryId(stat.categoryId) {};
};


/* ------------------------------- Storage & counters ----------------------------------- */
class StatCounters
{
private:

    smsc::logger::Logger    *logger;

    std::string     location;
    bool            bFileTM, bEmpty;
    tm              fileTM;
    File            statFile;

    SmsStat         statGen;
    Hash<SmsStat>   statBySme;
    Hash<RouteStat> statByRoute;

    static bool createDir(const std::string& dir);
    void dump(const uint8_t* buff, int buffLen, const tm& flushTM);

public:

    StatCounters(const std::string& l)
        : logger(Logger::getInstance("stat4log.storage")), location(l), bFileTM(false), bEmpty(true) {};
    ~StatCounters() {
        if (statFile.isOpened()) statFile.Close();
    }

    void flush(time_t flushTime);
    void incAccepted(const char* smeId, const char* routeId, signed long pId, signed long cId);
    void incDelivered(const char* smeId, const char* routeId, signed long pId, signed long cId);
};
bool StatCounters::createDir(const std::string& dir)
{
    if (mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) {
        if (errno == EEXIST) return false;
        throw Exception("Failed to create directory '%s'. Details: %s",
                        dir.c_str(), strerror(errno));
    }
    return true;
}

const uint16_t SMSC_STAT_VERSION_INFO  = 0x0001;
const char*    SMSC_STAT_HEADER_TEXT   = "SMSC.STAT";
const char*    SMSC_STAT_DIR_NAME_FORMAT  = "%04d-%02d";
const char*    SMSC_STAT_FILE_NAME_FORMAT = "%02d.rts";
const char*    SMSC_TRNS_FILE_NAME_FORMAT = "%02d.trs";

const int MAX_STACK_BUFFER_SIZE = 64*1024;

void StatCounters::dump(const uint8_t* buff, int buffLen, const tm& flushTM)
{
    smsc_log_info(logger, "Statistics dump for %02d-%02d %02d:%02d GMT",
                  flushTM.tm_mon+1, flushTM.tm_mday, flushTM.tm_hour, flushTM.tm_min);

    try
    {
        char dirName[128]; char fileName[128];
        sprintf(dirName, SMSC_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        sprintf(fileName, SMSC_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
        std::string fullPath = location; fullPath += '/'; fullPath += (const char*)dirName;
        std::string statPath = fullPath; statPath += '/'; statPath += (const char*)fileName;
        const char* statPathStr = statPath.c_str();

        if (!bFileTM || fileTM.tm_mon != flushTM.tm_mon || fileTM.tm_year != flushTM.tm_year)
        {
            StatCounters::createDir(fullPath); bFileTM = false;
            smsc_log_info(logger, "New dir '%s' created", dirName);
        }

        bool needHeader = false;
        if (!bFileTM || fileTM.tm_mday != flushTM.tm_mday)
        {
            // close old RTS file (if it was opened)
            if (statFile.isOpened()) statFile.Close();

            needHeader = true;
            if (File::Exists(statPathStr)) {
                needHeader = false;
                statFile.WOpen(statPathStr);
            } else {
                statFile.RWCreate(statPathStr);
            }

            fileTM = flushTM; bFileTM = true;
            smsc_log_info(logger, "%s file '%s' %s", (needHeader) ? "New":"Existed",
                           fileName, (needHeader) ? "created":"opened");
        }

        TmpBuf<uint8_t, MAX_STACK_BUFFER_SIZE> writeBuff(MAX_STACK_BUFFER_SIZE);
        if (needHeader) // create header (if new file created)
        {
            writeBuff.Append((uint8_t *)SMSC_STAT_HEADER_TEXT, strlen(SMSC_STAT_HEADER_TEXT));
            uint16_t version = htons(SMSC_STAT_VERSION_INFO);
            writeBuff.Append((uint8_t *)&version, sizeof(version));
        }
        uint32_t value32 = htonl(buffLen);
        writeBuff.Append((uint8_t *)&value32, sizeof(value32));
        writeBuff.Append((uint8_t *)buff, buffLen);
        writeBuff.Append((uint8_t *)&value32, sizeof(value32));
        statFile.Write((const void *)writeBuff, writeBuff.GetPos());
        statFile.Flush();

        smsc_log_info(logger, "Record dumped (%d bytes)", buffLen);
    }
    catch (std::exception& exc)
    {
        if (statFile.isOpened()) statFile.Close();
        bFileTM = false;
        throw;
    }
}
uint64_t toNetworkOrder(uint64_t value)
{
    uint64_t result = 0;
    unsigned char *ptr=(unsigned char *)&result;
    ptr[0]=(value>>56)&0xFF; ptr[1]=(value>>48)&0xFF;
    ptr[2]=(value>>40)&0xFF; ptr[3]=(value>>32)&0xFF;
    ptr[4]=(value>>24)&0xFF; ptr[5]=(value>>16)&0xFF;
    ptr[6]=(value>>8 )&0xFF; ptr[7]=(value    )&0xFF;
    return result;
}
void StatCounters::flush(time_t flushTime)
{
    if (bEmpty) return;
    tm flushTM; gmtime_r(&flushTime, &flushTM); flushTM.tm_min = 0;

    TmpBuf<uint8_t, MAX_STACK_BUFFER_SIZE> buff(MAX_STACK_BUFFER_SIZE);

    // General statistics dump
    uint8_t value8 = 0;
    value8 = (uint8_t)(flushTM.tm_hour); buff.Append((uint8_t *)&value8, sizeof(value8));
    value8 = (uint8_t)(flushTM.tm_min);  buff.Append((uint8_t *)&value8, sizeof(value8));

    int32_t value32 = 0;
    value32 = htonl(statGen.accepted);
    buff.Append((uint8_t *)&value32, sizeof(value32)); // accepted
    value32 = 0;
    buff.Append((uint8_t *)&value32, sizeof(value32)); // rejected
    value32 = htonl(statGen.delivered);
    buff.Append((uint8_t *)&value32, sizeof(value32)); // delivered
    value32 = 0;
    buff.Append((uint8_t *)&value32, sizeof(value32)); // failed
    buff.Append((uint8_t *)&value32, sizeof(value32)); // rescheduled
    buff.Append((uint8_t *)&value32, sizeof(value32)); // temporal
    buff.Append((uint8_t *)&value32, sizeof(value32)); // peak_i
    buff.Append((uint8_t *)&value32, sizeof(value32)); // peak_o

    buff.Append((uint8_t *)&value32, sizeof(value32)); // errors (count = 0)

    // Sme statistics dump
    value32 = statBySme.GetCount();
    value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
    statBySme.First();
    char* smeId = 0; SmsStat* smeStat = 0;
    while (statBySme.Next(smeId, smeStat))
    {
        if (!smeStat || !smeId || smeId[0] == '\0') continue;

        uint8_t smeIdLen = (uint8_t)strlen(smeId);
        buff.Append((uint8_t *)&smeIdLen, sizeof(smeIdLen));
        buff.Append((uint8_t *)smeId, smeIdLen);
        value32 = htonl(smeStat->accepted);  buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = 0;                         buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(smeStat->delivered); buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = 0;
        buff.Append((uint8_t *)&value32, sizeof(value32));
        buff.Append((uint8_t *)&value32, sizeof(value32));
        buff.Append((uint8_t *)&value32, sizeof(value32));
        buff.Append((uint8_t *)&value32, sizeof(value32));
        buff.Append((uint8_t *)&value32, sizeof(value32));

        buff.Append((uint8_t *)&value32, sizeof(value32)); // errors (count = 0)
        smeStat = 0;
    }

    // Route statistics dump
    value32 = statByRoute.GetCount();
    value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
    statByRoute.First();
    char* routeId = 0; RouteStat* routeStat = 0;
    while (statByRoute.Next(routeId, routeStat))
    {
        if (!routeStat || !routeId || routeId[0] == '\0')
            throw Exception("Invalid route stat record!");

        uint8_t routeIdLen = (uint8_t)strlen(routeId);
        buff.Append((uint8_t *)&routeIdLen, sizeof(routeIdLen));
        buff.Append((uint8_t *)routeId, routeIdLen);

        int64_t value64 = 0;
        value64 = toNetworkOrder(routeStat->providerId); buff.Append((uint8_t *)&value64, sizeof(value64));
        value64 = toNetworkOrder(routeStat->categoryId); buff.Append((uint8_t *)&value64, sizeof(value64));

        value32 = htonl(routeStat->accepted);  buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = 0;                           buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(routeStat->delivered); buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = 0;
        buff.Append((uint8_t *)&value32, sizeof(value32));
        buff.Append((uint8_t *)&value32, sizeof(value32));
        buff.Append((uint8_t *)&value32, sizeof(value32));
        buff.Append((uint8_t *)&value32, sizeof(value32));
        buff.Append((uint8_t *)&value32, sizeof(value32));

        buff.Append((uint8_t *)&value32, sizeof(value32)); // errors (count = 0)
        routeStat = 0;
    }

    dump(buff, buff.GetPos(), flushTM);
    statGen.Empty(); statBySme.Empty(); statByRoute.Empty(); bEmpty = true;
}
void StatCounters::incAccepted(const char* smeId, const char* routeId,
                               signed long pId, signed long cId)
{
    statGen.accepted++;
    if (smeId && smeId[0]) {
        SmsStat* stat = statBySme.GetPtr(smeId);
        if (stat) stat->accepted++;
        else statBySme.Insert(smeId, SmsStat(1, 0));
    }
    if (routeId && routeId[0]) {
        RouteStat* stat = statByRoute.GetPtr(routeId);
        if (stat) stat->accepted++;
        else statByRoute.Insert(routeId, RouteStat(1, 0, pId, cId));
    }
    bEmpty = false;
}
void StatCounters::incDelivered(const char* smeId, const char* routeId,
                                signed long pId, signed long cId)
{
    statGen.delivered++;
    if (smeId && smeId[0]) {
        SmsStat* stat = statBySme.GetPtr(smeId);
        if (stat) stat->delivered++;
        else statBySme.Insert(smeId, SmsStat(0, 1));
    }
    if (routeId && routeId[0]) {
        RouteStat* stat = statByRoute.GetPtr(routeId);
        if (stat) stat->delivered++;
        else statByRoute.Insert(routeId, RouteStat(0, 1, pId, cId));
    }
    bEmpty = false;
}
struct SubmitData
{
    std::string dstSmeId;
    std::string routeId;
    int64_t pId, cId;

    SubmitData(const std::string& smeId="", const std::string& rId="", int64_t p=-1, int64_t c=-1)
        : dstSmeId(smeId), routeId(rId), pId(p), cId(c) {};
};

/* ------------------------------- Main ----------------------------------- */

const char* LOG_PREFIX_PATTERN = "%c %02d-%02d %02d:%02d:%02d,%03d %03d%n";
const char* LOG_CAT_ST_PATTERN = "sms.trace: ";
const char* LOG_SBM_ST_PATTERN = "SBM: ";
const char* LOG_SBM_PR_PATTERN = "Id=%lld;seq=%d;%n";
const char* LOG_FWD_ST_PATTERN = "FWDDLV: ";
const char* LOG_FWD_PR_PATTERN = "msgId=%lld, seq number:%d;%n";
const char* LOG_DLVRSP_PATTERN = "DLVRSP: msgId=%lld;class=%n";

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
    logger = Logger::getInstance("smsc.stat.Stat4Log");

    SmeManager   smeManager;
    RouteManager routeManager;
    AliasManImpl aliasManager("store/aliases.bin");
    smsc::util::config::alias::AliasConfig aliasConfig;
    smsc::util::config::smeman::SmeManConfig smemanConfig;

    try
    {
        smsc_log_info(logger, "Loading SMSC configuration...");
        aliasConfig.load(findConfigFile("aliases.xml"));
        reloadAliases(&aliasManager, aliasConfig);
        smemanConfig.load(findConfigFile("sme.xml"));
        smsc::util::config::smeman::SmeManConfig::RecordIterator i = smemanConfig.getRecordIterator();
        using namespace smsc::util::regexp; RegExp re;
        while(i.hasRecord())
        {
          smsc::util::config::smeman::SmeRecord *rec;
          i.fetchNext(rec);
          SmeInfo si;
          if(rec->rectype==smsc::util::config::smeman::SMPP_SME)
          {
            si.typeOfNumber=rec->recdata.smppSme.typeOfNumber;
            si.numberingPlan=rec->recdata.smppSme.numberingPlan;
            si.interfaceVersion=rec->recdata.smppSme.interfaceVersion;
            si.rangeOfAddress=rec->recdata.smppSme.addrRange;
            si.systemType=rec->recdata.smppSme.systemType;
            si.password=rec->recdata.smppSme.password;
            si.systemId=rec->smeUid;
            si.timeout = rec->recdata.smppSme.timeout;
            si.wantAlias = rec->recdata.smppSme.wantAlias;
            //si.forceDC = rec->recdata.smppSme.forceDC;
            si.proclimit=rec->recdata.smppSme.proclimit;
            si.schedlimit=rec->recdata.smppSme.schedlimit;
            si.receiptSchemeName= rec->recdata.smppSme.receiptSchemeName;
            if(si.rangeOfAddress.length() && !re.Compile(si.rangeOfAddress.c_str(),OP_OPTIMIZE|OP_STRICT)) {
              smsc_log_warn(logger, "Failed to compile rangeOfAddress for sme %s",si.systemId.c_str());
            }
            //si.hostname=rec->recdata->smppSme.
            si.disabled=rec->recdata.smppSme.disabled;
            using namespace smsc::util::config::smeman;
            switch(rec->recdata.smppSme.mode)
            {
              case MODE_TX:si.bindMode=smeTX;break;
              case MODE_RX:si.bindMode=smeRX;break;
              case MODE_TRX:si.bindMode=smeTRX;break;
            };

            try { smeManager.addSme(si); } catch(...) {
              smsc_log_warn(logger, "UNABLE TO REGISTER SME:%s", si.systemId.c_str());
            }
          }
        }
        smsc::util::config::route::RouteConfig rc;
        rc.load(findConfigFile("routes.xml"));
        routeManager.assign(&smeManager);
        smsc::system::loadRoutes(&routeManager, rc);
        smsc_log_info(logger, "SMSC configuration loaded");
    }
    catch (std::exception& exc) {
        smsc_log_error(logger, "Failed to load SMSC configuration. Details: %s", exc.what());
        return -2;
    }

    // #################################### Log scanner part #####################################

    bool needCount = false;
    time_t lastTime = -1; time_t logTime = -1;
    tm logTM, lastTM; logTM.tm_year = 2005 - 1900;
    int LOG_level, TM_msec, TH_pid, seqNum;
    int catPatLen = strlen(LOG_CAT_ST_PATTERN);
    int sbmPatLen = strlen(LOG_SBM_ST_PATTERN);
    int fwdPatLen = strlen(LOG_FWD_ST_PATTERN);
    memset(&lastTM, 0, sizeof(lastTM));

    uint64_t msgId = 0;
    char oaBuff[128]; char daBuff[128]; char ddaBuff[128];
    char srcPrxBuff[512]; char dstPrxBuff[512];

    uint64_t totalAccepted = 0;
    uint64_t totalDelivered = 0;
    StatCounters statCounters(".");

    XHash<uint64_t, bool>       countedMsgId;
    XHash<uint64_t, SubmitData> submitByMsgId;

    int resultCode = 0;
    for (int i=0; i<logFiles.Count(); i++)
    {
        File logFile;
        std::string logFileName = logFiles[i];
        try { logFile.ROpen(logFileName.c_str()); }
        catch (FileException& fexc) {
            smsc_log_error(logger, "Failed to open log file '%s'. Details: %s",
                           logFileName.c_str(), fexc.what());
            continue;
        }
        smsc_log_info(logger, "Scanning log file %s", logFileName.c_str());

        std::string logLine;
        while (logFile.ReadLine(logLine))
        {
            const char* str = logLine.c_str();
            int bytesRead = 0;
            if (sscanf(str, LOG_PREFIX_PATTERN, &LOG_level, &logTM.tm_mday, &logTM.tm_mon,
                       &logTM.tm_hour, &logTM.tm_min, &logTM.tm_sec, &TM_msec, &TH_pid, &bytesRead) != 8) continue;

            logTime = convertTime(logTM); // from local time
            if (tillDateFilter > 0 && logTime >= tillDateFilter) break; // TODO: dump scanned counters
            needCount = (fromDateFilter <= 0 || logTime >= fromDateFilter);

            if (lastTime <= 0 ||
                lastTM.tm_hour != logTM.tm_hour || lastTM.tm_mday != logTM.tm_mday ||
                lastTM.tm_mon != logTM.tm_mon || lastTM.tm_year != logTM.tm_year)
            {
                if (needCount && lastTime > 0) statCounters.flush(lastTime); // dump counters to file & cleanup it
                lastTime = logTime; lastTM = logTM;
            }

            str += bytesRead;
            while (*str && isspace(*str)) str++;
            if (strncmp(str, LOG_CAT_ST_PATTERN, catPatLen) != 0) continue; // 'sms.trace: ' not matched
            str += catPatLen;
            while (*str && isspace(*str)) str++;

            const char* sub = 0;
            if (strncmp(str, LOG_SBM_ST_PATTERN, sbmPatLen) == 0) // 'SBM: ' matched
            {
// ������ ��� ������� !!!!
// I SBM: submit ok, seqnum=4187769 Id=11794273310;seq=130463;oa=.1.1.79138923688;da=.5.0.ussd:102;dda=.0.1.102;   srcprx=MAP_PROXY; dstprx=dbSme
// I SBM: dest sme not connected   Id=11794312619;seq=29963;oa=.1.1.79137500058;da=.0.9.0850;srcprx=upcs_gw; dstprx=espp

                bool dstSmeMsg = false;
                str += sbmPatLen;
                if (!strstr(str, "submit ok, seqnum=")) {
                     if (!strstr(str, "dest sme not connected")) continue;
                     dstSmeMsg = true;
                }
                if (!(sub = strstr(str, "Id="))) continue;
                if (sub != str) str = sub; // skip message (submit ok | dest sme not connected)
                if (sscanf(str, LOG_SBM_PR_PATTERN, &msgId, &seqNum, &bytesRead) != 2) continue;
                str += bytesRead;
                if (strncmp(str, "oa=", 3)) continue; str += 3;
                if (!(sub = strstr(str, ";da="))) continue;
                strncpy(oaBuff, str, sub-str); oaBuff[sub-str] = 0; str = sub + 4;

                if (!dstSmeMsg) {
                    if (!(sub = strstr(str, ";dda="))) continue;
                    strncpy(daBuff, str, sub-str); daBuff[sub-str] = 0;
                    if (!(sub = strstr(str, ";srcprx="))) continue;
                }
                else {
                    if (!(sub = strstr(str, ";srcprx="))) continue;
                    strncpy(daBuff, str, sub-str); daBuff[sub-str] = 0;
                }
                str = sub + 8;
                if (!(sub = strstr(str, ";dstprx="))) continue;
                strncpy(srcPrxBuff, str, sub-str); srcPrxBuff[sub-str] = 0;
                //strcpy(dstPrxBuff, sub + 8);

                //printf("SUBMIT >>%s msgId=%lld, seq=%ld, oa='%s', da='%s'\n", msgId, seqNum, oaBuff, daBuff);
                try
                {
                    RouteInfo routeInfo; //int tmpIdx = 0; SmeProxy* tmpSmeProxy = 0;
                    int srcPrxIdx = smeManager.lookup(srcPrxBuff);
                    Address oaAddress(oaBuff); Address doaAddress;
                    Address daAddress(daBuff); Address ddaAddress;
                    bool oaAliasFound = aliasManager.AliasToAddress(oaAddress, doaAddress);
                    bool daAliasFound = aliasManager.AliasToAddress(daAddress, ddaAddress);
                    RouteResult rr;
                    bool routeFound = routeManager.lookup(srcPrxIdx,
                                                          oaAliasFound ? doaAddress:oaAddress,
                                                          daAliasFound ? ddaAddress:daAddress,
                                                          rr);
                    if (!routeFound) {
                        smsc_log_warn(logger, "SBM: msgId=%lld No route for oa=%s, da=%s, srcprx=%s (%d)",
                                      msgId, oaBuff, daBuff, srcPrxBuff, srcPrxIdx);
                        continue;
                        /*for (std::vector<std::string>::iterator t=traceV.begin();t!=traceV.end();t++) {
                            smsc_log_warn(logger, "SBM: %s alias=%s", t->c_str(), aliasFound ? "yes":"no");
                        }
                        return -5;*/
                    }
                    routeInfo=rr.info;

                    SubmitData sd(routeInfo.smeSystemId, routeInfo.routeId,
                                  routeInfo.providerId, routeInfo.categoryId);
                    SubmitData* psd = submitByMsgId.GetPtr(msgId);
                    if (!psd) submitByMsgId.Insert(msgId, sd);
                    else smsc_log_warn(logger, "SBM: msgId=%lld already present", msgId);

                    if (needCount) {
                        statCounters.incAccepted(srcPrxBuff, routeInfo.routeId.c_str(),
                                                 routeInfo.providerId, routeInfo.categoryId);
                        totalAccepted++;
                    }
                }
                catch (std::exception& sexc) {
                  smsc_log_warn(logger, "SBM: Failed to lookup route for msgId=%lld, "
                                "oa=%s, da=%s, srcprx=%s. Details: %s",
                                msgId, oaBuff, daBuff, srcPrxBuff,sexc.what());
                  continue;
                }
            }
            else if (sscanf(str, LOG_DLVRSP_PATTERN, &msgId, &bytesRead) == 1) // 'DLVRSP: ' matched
            {
// I DLVRSP: msgId=11793779293;class=TEMP ERROR;st=1179;oa=.0.1.0001302;da=.0.1.79132873419;dda=.1.1.79132873419
// I DLVRSP: msgId=11794272650;class=OK;st=0;oa=.1.1.79139869990;da=.1.1.79137242867;dda=.1.1.79137242867
// class ="OK"|"RESCHEDULEDNOW"|"TEMP ERROR"|"PERM ERROR",
                //int st = 0;
                str += bytesRead;
                if (*str != 'O') continue; // ! class=OK;
                //if (!(sub = strstr(str, ";st="))) continue;

                /*str = sub + 4;
                if (sscanf(str, "%d", &st) != 1) continue;
                if (!(sub = strstr(str, ";oa="))) continue; str = sub + 4;
                if (!(sub = strstr(str, ";da="))) continue;
                strncpy(oaBuff, str, sub-str); oaBuff[sub-str] = 0; str = sub + 4;
                if (!(sub = strstr(str, ";dda="))) continue;
                strncpy(daBuff, str, sub-str); daBuff[sub-str] = 0; str = sub + 5;
                strcpy(ddaBuff, str);*/

                //printf("DLVRSP >> msgId=%lld, st=%d, oa='%s', da='%s', dda='%s'\n",
                //       msgId, st, oaBuff, daBuff, ddaBuff);

                bool* counted = countedMsgId.GetPtr(msgId);
                if (countedMsgId.Exists(msgId)) continue; // already counted
                SubmitData* sd = submitByMsgId.GetPtr(msgId);
                if (!sd) {
                    if (needCount) smsc_log_warn(logger, "DLVR: msgId=%lld SUBMIT not found", msgId);
                    continue;
                }
                if (needCount) {
                    statCounters.incDelivered(sd->dstSmeId.c_str(), sd->routeId.c_str(), sd->pId, sd->cId);
                    totalDelivered++;
                    submitByMsgId.Delete(msgId);
                    countedMsgId.Insert(msgId, true);
                }
            }
        }
    }

    if (needCount && lastTime > 0) statCounters.flush(lastTime); // flush the rest of counters

    smsc_log_info(logger, "Log files scanned. Total %lld accepted, %lld delivered",
                  totalAccepted, totalDelivered);
    return resultCode;
}
