
#define _FILE_OFFSET_BITS 64  // Win32
#define __USE_FILE_OFFSET64   // X     TODO: Move it to makefile

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <smpp/smpp_structures.h>
#include <util/csv/CSVFileEncoder.h>

#include "Uint64Converter.h"
#include "FileStorage.h"

/* Static check for 64bit positions for files */
template <bool cnd> struct StaticCheck {}; 
template <>  struct StaticCheck<true>  {}; 
template <>  struct StaticCheck<false> { private: StaticCheck(); }; 
static StaticCheck< sizeof(fpos_t)==8 > staticChec; 

namespace smsc { namespace store
{

using namespace smsc::util::csv;
using smsc::smpp::DataCoding;

const unsigned SMSC_MIN_BILLING_INTERVAL = 10;
const unsigned SMSC_MIN_ARCHIVE_INTERVAL = 5;

const char* SMSC_LAST_BILLING_FILE_EXTENSION = "lst";
const char* SMSC_PREV_BILLING_FILE_EXTENSION = "csv";
const char* SMSC_LAST_ARCHIVE_FILE_EXTENSION = "rec";
const char* SMSC_PREV_ARCHIVE_FILE_EXTENSION = "arc";
const char* SMSC_ERRF_ARCHIVE_FILE_EXTENSION = "err";

const char* SMSC_BILLING_FILE_NAME_PATTERN = "%04d%02d%02d_%02d%02d%02d";
const char* SMSC_ARCHIVE_FILE_NAME_PATTERN = "%04d%02d%02d_%02d%02d%02d";
const char* SMSC_PERSIST_FILE_NAME_PATTERN = "%02d.arc";
const char* SMSC_PERSIST_DIR_NAME_PATTERN  = "%04d%02d%02d";

const uint16_t SMSC_ARCHIVE_VERSION_INFO = 0x0001;
const char*    SMSC_ARCHIVE_HEADER_TEXT  = "SMSC.ARC";

void FileStorage::findEntries(std::string location, Array<std::string>& entries, 
                              bool files, const char* ext)
{
    int extFileLen  = 0;
    const char* locationStr = location.c_str();

    if (files) {
        if (!ext || !ext[0]) return;
        extFileLen = strlen(ext)+1;
    }
    
    DIR *locationDir = 0;
    if (!(locationDir = opendir(locationStr))) {
        Exception exc("Failed to open directory '%s'. Details: %s", locationStr, strerror(errno));
        throw StorageException(exc.what());
    }
    
    //printf("Max name len=%d\n", pathconf(locationStr, _PC_NAME_MAX));
    TmpBuf<char, 1024> entryGuard(sizeof(struct dirent)+pathconf(locationStr, _PC_NAME_MAX));
    char* entry = entryGuard.get();
    struct dirent* pentry = 0;

    while (locationDir)
    {
        errno = 0;
        int result = readdir_r(locationDir, (struct dirent *)entry, &pentry);
        if (!result && pentry != NULL)
        {
            std::string entryName = location; entryName += '/'; entryName += pentry->d_name;
            struct stat description;
            if (stat(entryName.c_str(), &description) != 0) {
                Exception exc("Failed to obtain '%s' entry info. Details: %s",
                              entryName.c_str(), strerror(errno));
                if (locationDir) closedir(locationDir);
                throw StorageException(exc.what());
            }
            //printf("%s\tmode:%d\n", pentry->d_name, description.st_mode);
            if (files) {                                        // file
                if (!(description.st_mode & S_IFDIR)) {    
                    int fileNameLen = strlen(pentry->d_name);
                    if (fileNameLen > extFileLen) {
                        const char* extPos = pentry->d_name+(fileNameLen-extFileLen);
                        if ((*extPos == '.') && !strcmp(extPos+1, ext)) entries.Push(pentry->d_name);
                    }
                }
            } else if (description.st_mode & S_IFDIR) {           // directory
                if (strcmp(pentry->d_name, ".") && strcmp(pentry->d_name, ".."))
                    entries.Push(pentry->d_name);
            }
        }
        else
        {
            if (errno == 0) break;
            Exception exc("Failed to scan directory '%s' contents. Details: %s",
                          locationStr, strerror(errno));
            if (locationDir) closedir(locationDir);
            throw StorageException(exc.what());
        }
    }

    if (locationDir) closedir(locationDir);
}

void FileStorage::findFiles(std::string location, const char* ext, Array<std::string>& files)
{
    FileStorage::findEntries(location, files, true, ext);    
}
void FileStorage::findDirs (std::string location, Array<std::string>& dirs)
{
    FileStorage::findEntries(location, dirs, false, 0);    
}

void FileStorage::deleteFile(std::string location, std::string fileName)
{
    std::string fullPath = location; fullPath +='/'; fullPath += fileName;
    if (remove(fullPath.c_str()) != 0) {
        Exception exc("Failed to remove file '%s'. Details: %s", fullPath.c_str(), strerror(errno));
        throw StorageException(exc.what());
    }
}
void FileStorage::rollErrorFile(std::string location, std::string fileName)
{
    std::string fullOldFile = location; fullOldFile += '/'; fullOldFile += fileName;
    std::string::size_type extpos = fileName.find_last_of('.');
    if (extpos != fileName.npos) fileName.erase(extpos);
    std::string fullNewFile = location; fullNewFile += '/'; fullNewFile += fileName;
    fullNewFile += '.'; fullNewFile += SMSC_ERRF_ARCHIVE_FILE_EXTENSION;
    if (rename(fullOldFile.c_str(), fullNewFile.c_str()) != 0) {
        Exception exc("Failed to rename file '%s' to '%s'. Details: %s",
                      fullOldFile.c_str(), fullNewFile.c_str(), strerror(errno));
        throw StorageException(exc.what());
    }
}
void FileStorage::rollFileExtension(std::string location, const char* fileName, bool bill)
{
    std::string fullOldFile = location; fullOldFile += '/'; fullOldFile += fileName; fullOldFile += '.'; 
    std::string fullNewFile = location; fullNewFile += '/'; fullNewFile += fileName; fullNewFile += '.';
    
    fullOldFile += (bill) ? SMSC_LAST_BILLING_FILE_EXTENSION : SMSC_LAST_ARCHIVE_FILE_EXTENSION;
    fullNewFile += (bill) ? SMSC_PREV_BILLING_FILE_EXTENSION : SMSC_PREV_ARCHIVE_FILE_EXTENSION;
    if (rename(fullOldFile.c_str(), fullNewFile.c_str()) != 0) {
        Exception exc("Failed to rename file '%s' to '%s'. Details: %s",
                      fullOldFile.c_str(), fullNewFile.c_str(), strerror(errno));
        throw StorageException(exc.what());
    }
}
bool FileStorage::createDir(std::string dir)
{
    if (mkdir(dir.c_str(), S_IRWXU|S_IXUSR|S_IROTH|S_IWOTH) != 0) { // TODO: define mode
        if (errno == EEXIST) return false;
        Exception exc("Failed to create directory '%s'. Details: %s",
                      dir.c_str(), strerror(errno));
        throw StorageException(exc.what());
    }
    return true;
}

bool FileStorage::read(void* data, size_t size)
{
    if (!storageFile) return false;
    if (fread(data, size, 1, storageFile) != 1) {
        if (feof(storageFile)) {
            clearerr(storageFile);
            return false;
        }
        int error = ferror(storageFile);
        Exception exc("Failed to read file. Details: %s", strerror(error));
        fclose(storageFile); storageFile = 0;
        throw StorageException(exc.what());
    }
    return true;
}
void FileStorage::write(const void* data, size_t size)
{
    if (storageFile && fwrite(data, size, 1, storageFile) != 1) {
        int error = ferror(storageFile);
        Exception exc("Failed to write file. Details: %s", strerror(error));
        fclose(storageFile); storageFile = 0;
        throw StorageException(exc.what());
    }
}
void FileStorage::flush()
{
    if (storageFile && fflush(storageFile)) {
        int error = ferror(storageFile);
        Exception exc("Failed to flush file. Details: %s", strerror(error));
        fclose(storageFile); storageFile = 0;
        throw StorageException(exc.what());
    }
}
void FileStorage::close()
{
    MutexGuard guard(storageFileLock);
    if (storageFile) {
        fclose(storageFile); storageFile = 0;
    }
}

void FileStorage::getPos(fpos_t* pos)
{
    if (storageFile && fgetpos(storageFile, pos) != 0) {
        int error = ferror(storageFile);
        Exception exc("Failed to get position in file. Details: %s", strerror(error));
        fclose(storageFile); storageFile = 0;
        throw StorageException(exc.what());
    }
}
void FileStorage::setPos(const fpos_t* pos)
{
    if (storageFile && fsetpos(storageFile, pos) != 0) {
        int error = ferror(storageFile);
        Exception exc("Failed to set position in file. Details: %s", strerror(error));
        fclose(storageFile); storageFile = 0;
        throw StorageException(exc.what());
    }
}

void RollingStorage::init(Manager& config, bool bill)
{
    storageLocation = config.getString(bill ? "MessageStore.billingDir":"MessageStore.archiveDir");
    int ci = config.getInt(bill ? "MessageStore.billingInterval":"MessageStore.archiveInterval");
    int minInterval = (bill ? SMSC_MIN_BILLING_INTERVAL:SMSC_MIN_ARCHIVE_INTERVAL);
    if (ci < minInterval) {
        throw ConfigException("Parameter '%sInterval' should be more than %u seconds",
                              (bill ? "billing":"archive"), minInterval);
    }
    else storageInterval = ci;
    
    Array<std::string> files;
    const char* extension = bill ? SMSC_LAST_BILLING_FILE_EXTENSION:SMSC_LAST_ARCHIVE_FILE_EXTENSION;
    FileStorage::findFiles(storageLocation, extension, files);
    int extLen = strlen(extension)+1;

    for (int i=0; i<files.Count(); i++)
    {
        std::string file = files[i];
        int fileNameLen = file.length();
        const char* fileNameStr = file.c_str();
        smsc_log_debug(log, "Found old %s file: %s", (bill ? "billing":"archive"), fileNameStr);

        TmpBuf<char, 1024> fileNameGuard(fileNameLen+1);
        char* fileName = fileNameGuard.get();
        strncpy(fileName, fileNameStr, fileNameLen-extLen);
        fileName[fileNameLen-extLen] = '\0';
        FileStorage::rollFileExtension(storageLocation, fileName, bill);
    }
}

bool RollingStorage::create(bool bill)
{
    if (storageFile) return false;
    
    time_t current = time(NULL);
    tm dt; gmtime_r(&current, &dt);
    sprintf(storageFileName, bill ? SMSC_BILLING_FILE_NAME_PATTERN:SMSC_ARCHIVE_FILE_NAME_PATTERN,
            dt.tm_year+1900, dt.tm_mon+1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);

    std::string fullFilePath = storageLocation;
    fullFilePath += '/'; fullFilePath += (const char*)storageFileName; fullFilePath += '.';
    fullFilePath += (bill ? SMSC_LAST_BILLING_FILE_EXTENSION:SMSC_LAST_ARCHIVE_FILE_EXTENSION);
    const char* fullFilePathStr = fullFilePath.c_str();
    storageFile = fopen(fullFilePathStr, "r");

    bool needFile = true;
    if (storageFile) { // file exists
        fclose(storageFile); storageFile = 0;
        needFile = false;
    }

    storageFile = fopen(fullFilePathStr, "ab+");
    if (!storageFile) {
        Exception exc("Failed to create %s file '%s'. Details: %s",
                      (bill ? "billing":"archive"), fullFilePathStr, strerror(errno));
        throw StorageException(exc.what());
    }
    if (fseek(storageFile, 0, SEEK_END)) {
        int error = ferror(storageFile);
        Exception exc("Failed to seek EOF. Details: %s", strerror(error));
        fclose(storageFile); storageFile = 0;
        throw StorageException(exc.what());
    }

    return needFile;
}

void RollingStorage::roll(bool bill)
{
    MutexGuard guard(storageFileLock);
    if (storageFile) {
        fclose(storageFile); storageFile = 0;
        FileStorage::rollFileExtension(storageLocation, storageFileName, bill);
    }
}

/*
MSG_ID                    -- msg id
RECORD_TYPE               -- 0 SMS, 1 Diverted SMS
MEDIA_TYPE                -- 0 SMS text, 1 SMS binary
BEARER_TYPE               -- 0 SMS, 1 USSD
SUBMIT                    -- submit time
FINALIZED                 -- finalized time
STATUS                    -- LAST_RESULT from SMS_MSG
SRC_ADDR                  -- OA  (.1.1.7865765 format)
SRC_IMSI                  -- SRC IMSI
SRC_MSC                   -- SRC MSC
SRC_SME_ID                --
DST_ADDR                  -- DDA (.1.1.7865765 format)
DST_IMSI                  -- DST IMSI
DST_MSC                   -- DST MSC
DST_SME_ID                --
DIVERTED_FOR              -- message originally was for DIVERTED_FOR address, but was delivered to DST_ADDR
ROUTE_ID                  -- ROUTE_ID from SMS_MSG
SERVICE_ID                -- SERVICE_ID from SMS_MSG
USER_MSG_REF
DATA_LENGTH               -- text or binary length (add it to SMS_MSG insteed of TXT_LENGTH)
*/
void BillingStorage::createRecord(SMSId id, SMS& sms)
{
    MutexGuard guard(storageFileLock);

    create();

    std::string out = "";
    bool isDiverted = sms.hasStrProperty(Tag::SMSC_DIVERTED_TO);
    bool isBinary   = sms.hasIntProperty(Tag::SMPP_DATA_CODING) ?
                     (sms.getIntProperty(Tag::SMPP_DATA_CODING) == DataCoding::BINARY) : false;

    CSVFileEncoder::addUint64(out, id);
    CSVFileEncoder::addUint8 (out, isDiverted ? 1:0);
    CSVFileEncoder::addUint8 (out, isBinary   ? 1:0);
    CSVFileEncoder::addUint8 (out, sms.hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) ? 1:0);
    CSVFileEncoder::addDateTime(out, sms.submitTime);
    CSVFileEncoder::addDateTime(out, sms.lastTime);
    CSVFileEncoder::addUint32(out, sms.lastResult);

    std::string oa = sms.originatingAddress.toString();
    CSVFileEncoder::addString(out, oa.c_str());
    CSVFileEncoder::addString(out, sms.originatingDescriptor.imsi);
    CSVFileEncoder::addString(out, sms.originatingDescriptor.msc);
    //CSVFileEncoder.addUint32(out, sms.originatingDescriptor.sme);
    CSVFileEncoder::addString(out, sms.srcSmeId);

    std::string dda = sms.dealiasedDestinationAddress.toString();
    CSVFileEncoder::addString(out, dda.c_str());
    CSVFileEncoder::addString(out, sms.destinationDescriptor.imsi);
    CSVFileEncoder::addString(out, sms.destinationDescriptor.msc);
    //CSVFileEncoder.addUint32(out, sms.destinationDescriptor.sme);
    CSVFileEncoder::addString(out, sms.dstSmeId);

    if (isDiverted) {
        std::string divertedTo = sms.getStrProperty(Tag::SMSC_DIVERTED_TO);
        CSVFileEncoder::addString(out, divertedTo.c_str());
    }
    else CSVFileEncoder::addString(out, 0);
    CSVFileEncoder::addString(out, sms.routeId);
    CSVFileEncoder::addInt32 (out, sms.serviceId);
    if (sms.hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE))
        CSVFileEncoder::addUint32(out, sms.getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
    else
        CSVFileEncoder::addSeparator(out);
    CSVFileEncoder::addUint32(out, sms.messageBody.getShortMessageLength(), true);

    write(out.c_str(), out.length());
    flush();
}

/*
ID             NUMBER(22)
ST             NUMBER(3)
SUBMIT_TIME    DATE
VALID_TIME     DATE
ATTEMPTS       NUMBER(22)
LAST_RESULT    NUMBER(22)
LAST_TRY_TIME  DATE
NEXT_TRY_TIME  DATE
OA             VARCHAR2(30)
DA             VARCHAR2(30)
DDA            VARCHAR2(30)
MR             NUMBER(5)
SVC_TYPE       VARCHAR2(6)
DR             NUMBER(3)
BR             NUMBER(3)
SRC_MSC        VARCHAR2(21)
SRC_IMSI       VARCHAR2(21)
SRC_SME_N      NUMBER(22)
DST_MSC        VARCHAR2(21)
DST_IMSI       VARCHAR2(21)
DST_SME_N      NUMBER(22)
ROUTE_ID       VARCHAR2(32)
SVC_ID         NUMBER(22)
PRTY           NUMBER(22)
SRC_SME_ID     VARCHAR2(15)
DST_SME_ID     VARCHAR2(15)
TXT_LENGTH     NUMBER(10)
BODY_LEN       NUMBER(10)
BODY           RAW(1500)
*/
void FileStorage::save(SMSId id, SMS& sms, fpos_t* pos /*= 0 (no getPos) */)
{
    uint8_t smsState = (uint8_t)sms.state;
    std::string oa  = sms.originatingAddress.toString();
    std::string da  = sms.destinationAddress.toString();
    std::string dda = sms.dealiasedDestinationAddress.toString();
    int8_t oaSize   = oa.length();
    int8_t daSize   = da.length();
    int8_t ddaSize  = dda.length();
    int8_t svcSize    = strlen(sms.eServiceType);
    int8_t odMscSize  = strlen(sms.originatingDescriptor.msc);
    int8_t odImsiSize = strlen(sms.originatingDescriptor.imsi);
    int8_t ddMscSize  = strlen(sms.destinationDescriptor.msc);
    int8_t ddImsiSize = strlen(sms.destinationDescriptor.imsi);
    int8_t routeSize  = strlen(sms.routeId);
    int8_t srcSmeSize = strlen(sms.srcSmeId);
    int8_t dstSmeSize = strlen(sms.dstSmeId);
    int32_t bodyBufferLen = sms.messageBody.getBufferLength();
    int32_t textLen       = 0;

    uint32_t recordSize = sizeof(id)+sizeof(smsState)+sizeof(sms.submitTime)+sizeof(sms.validTime)+
        sizeof(sms.attempts)+sizeof(sms.lastResult)+sizeof(sms.lastTime)+sizeof(sms.nextTime)+
        sizeof(oaSize)+oaSize+sizeof(daSize)+daSize+sizeof(ddaSize)+ddaSize+sizeof(sms.messageReference)+
        sizeof(svcSize)+svcSize+sizeof(sms.deliveryReport)+sizeof(sms.billingRecord)+
        sizeof(odMscSize)+odMscSize+sizeof(odImsiSize)+odImsiSize+sizeof(sms.originatingDescriptor.sme)+
        sizeof(ddMscSize)+ddMscSize+sizeof(ddImsiSize)+ddImsiSize+sizeof(sms.destinationDescriptor.sme)+
        sizeof(routeSize)+routeSize+sizeof(sms.serviceId)+sizeof(sms.priority)+
        sizeof(srcSmeSize)+srcSmeSize+sizeof(dstSmeSize)+dstSmeSize+
        sizeof(bodyBufferLen)+bodyBufferLen;

    if (pos) FileStorage::getPos(pos);

    uint32_t writeBufferSize = recordSize+sizeof(recordSize)*2;
    TmpBuf<char, 2048> writeBufferGuard(writeBufferSize);
    char* writeBuffer = writeBufferGuard.get(); char* position = writeBuffer;
    
    recordSize = htonl(recordSize);
    memcpy(position, &recordSize, sizeof(recordSize)); position+=sizeof(recordSize);
    SMSId idd = Uint64Converter::toNetworkOrder(id);
    memcpy(position, &idd, sizeof(idd)); position+=sizeof(idd);
    memcpy(position, &smsState, sizeof(smsState)); position+=sizeof(smsState);
    
    time_t writeTime = htonl(sms.submitTime);
    memcpy(position, &writeTime, sizeof(writeTime)); position+=sizeof(writeTime);
    writeTime = htonl(sms.validTime);
    memcpy(position, &writeTime, sizeof(writeTime)); position+=sizeof(writeTime);
    writeTime = htonl(sms.lastTime);
    memcpy(position, &writeTime, sizeof(writeTime)); position+=sizeof(writeTime);
    writeTime = htonl(sms.nextTime);
    memcpy(position, &writeTime, sizeof(writeTime)); position+=sizeof(writeTime);
    
    uint32_t attempts = htonl(sms.attempts);
    memcpy(position, &attempts, sizeof(attempts)); position+=sizeof(attempts);
    uint32_t lastResult = htonl(sms.lastResult);
    memcpy(position, &lastResult, sizeof(lastResult)); position+=sizeof(lastResult);
    
    memcpy(position, &oaSize, sizeof(oaSize));   position+=sizeof(oaSize);
    if (oaSize > 0)  { memcpy(position, oa.c_str(), oaSize);   position+=oaSize;  }
    memcpy(position, &daSize, sizeof(daSize));   position+=sizeof(daSize);    
    if (daSize > 0)  { memcpy(position, da.c_str(), daSize);   position+=daSize;  }
    memcpy(position, &ddaSize, sizeof(ddaSize)); position+=sizeof(ddaSize);
    if (ddaSize > 0) { memcpy(position, dda.c_str(), ddaSize); position+=ddaSize; }
    
    uint16_t mr = htons(sms.messageReference);
    memcpy(position, &mr, sizeof(mr)); position+=sizeof(mr);
    memcpy(position, &svcSize, sizeof(svcSize)); position+=sizeof(svcSize);
    if (svcSize > 0) { memcpy(position,  sms.eServiceType, svcSize); position+=svcSize; }
    memcpy(position, &sms.deliveryReport, sizeof(sms.deliveryReport)); position+=sizeof(sms.deliveryReport);
    memcpy(position, &sms.billingRecord, sizeof(sms.billingRecord)); position+=sizeof(sms.billingRecord);

    memcpy(position, &odMscSize, sizeof(odMscSize));   position+=sizeof(odMscSize);
    if (odMscSize > 0)  { memcpy(position, sms.originatingDescriptor.msc, odMscSize);   position+=odMscSize; }
    memcpy(position, &odImsiSize, sizeof(odImsiSize)); position+=sizeof(odImsiSize);
    if (odImsiSize > 0) { memcpy(position, sms.originatingDescriptor.imsi, odImsiSize); position+=odImsiSize; }
    uint32_t odSme = htonl(sms.originatingDescriptor.sme);
    memcpy(position, &odSme, sizeof(odSme)); position+=sizeof(odSme);
    memcpy(position, &ddMscSize, sizeof(ddMscSize));   position+=sizeof(ddMscSize);
    if (ddMscSize > 0)  { memcpy(position, sms.destinationDescriptor.msc, ddMscSize);   position+=ddMscSize; }
    memcpy(position, &ddImsiSize, sizeof(ddImsiSize)); position+=sizeof(ddImsiSize);
    if (ddImsiSize > 0) { memcpy(position, sms.destinationDescriptor.imsi, ddImsiSize); position+=ddImsiSize; }
    uint32_t ddSme = htonl(sms.destinationDescriptor.sme);
    memcpy(position, &ddSme, sizeof(ddSme)); position+=sizeof(ddSme);
    
    memcpy(position, &routeSize, sizeof(routeSize)); position+=sizeof(routeSize);
    if (routeSize > 0)  { memcpy(position,  sms.routeId, routeSize); position+=routeSize; }
    int32_t svcId = (int32_t)htonl(sms.serviceId);
    memcpy(position, &svcId, sizeof(svcId)); position+= sizeof(svcId);
    int32_t priority = (int32_t)htonl(sms.priority);
    memcpy(position, &priority, sizeof(priority)); position+=sizeof(priority);
    memcpy(position, &srcSmeSize, sizeof(srcSmeSize)); position+=sizeof(srcSmeSize);
    if (srcSmeSize > 0) { memcpy(position,  sms.srcSmeId, srcSmeSize); position+=srcSmeSize; }
    memcpy(position, &dstSmeSize, sizeof(dstSmeSize)); position+=sizeof(dstSmeSize);
    if (dstSmeSize > 0) { memcpy(position,  sms.dstSmeId, dstSmeSize); position+=dstSmeSize; }

    textLen = (int32_t)htonl(bodyBufferLen);
    memcpy(position, &textLen, sizeof(textLen)); position+=sizeof(textLen);
    if (bodyBufferLen > 0) {
        uint8_t* bodyBuffer = sms.messageBody.getBuffer();
        memcpy(position,  bodyBuffer, bodyBufferLen); position+=bodyBufferLen;
    }
    
    memcpy(position, &recordSize, sizeof(recordSize)); position+=sizeof(recordSize);

    write(writeBuffer, writeBufferSize);
    flush();
}

bool FileStorage::load(SMSId& id, SMS& sms, const fpos_t* pos /*= 0 (no setPos) */)
{
    uint8_t  smsState = 0;
    uint32_t recordSize1 = 0; uint32_t recordSize2 = 0;
    int8_t oaSize    = 0; int8_t daSize = 0; int8_t ddaSize  = 0; int8_t svcSize  = 0;
    int8_t odMscSize = 0; int8_t odImsiSize = 0; 
    int8_t ddMscSize = 0; int8_t ddImsiSize = 0;
    int8_t routeSize = 0; int8_t srcSmeSize = 0; int8_t dstSmeSize = 0;
    int32_t textLen  = 0; int32_t bodyBufferLen = 0;

    if (pos) FileStorage::setPos(pos);

    try
    {
        if (!read(&recordSize1, sizeof(recordSize1))) return false;
        else recordSize1 = ntohl(recordSize1);

        TmpBuf<char, 2048> readBufferGuard(recordSize1);
        char* readBuffer = readBufferGuard.get();
        if (!read(readBuffer, recordSize1)) return false;
        
        if (!read(&recordSize2, sizeof(recordSize2))) return false;
        else recordSize2 = ntohl(recordSize2);

        if (recordSize1 != recordSize2) 
            throw Exception("Inconsistent archive file rs1=%u, rs2=%u", recordSize1, recordSize2);
        
        char* position = readBuffer;
        memcpy(&id, position, sizeof(id)); position+=sizeof(id);
        id = Uint64Converter::toHostOrder(id);
        memcpy(&smsState, position, sizeof(smsState)); position+=sizeof(smsState);
        sms.state = (smsc::sms::State)smsState;

        memcpy(&sms.submitTime, position, sizeof(sms.submitTime)); position+=sizeof(sms.submitTime);
        sms.submitTime = ntohl(sms.submitTime);
        memcpy(&sms.validTime,  position, sizeof(sms.validTime) ); position+=sizeof(sms.validTime);
        sms.validTime  = ntohl(sms.validTime);
        memcpy(&sms.lastTime,   position, sizeof(sms.lastTime)  ); position+=sizeof(sms.lastTime);
        sms.lastTime   = ntohl(sms.lastTime);
        memcpy(&sms.nextTime,   position, sizeof(sms.nextTime)  ); position+=sizeof(sms.nextTime);
        sms.nextTime   = ntohl(sms.nextTime);

        memcpy(&sms.attempts, position, sizeof(sms.attempts)); position+=sizeof(sms.attempts);
        sms.attempts = ntohl(sms.attempts);
        memcpy(&sms.lastResult, position, sizeof(sms.lastResult)); position+=sizeof(sms.lastResult);
        sms.lastResult = ntohl(sms.lastResult);
        
        char strBuff[1024];
        memcpy(&oaSize, position, sizeof(oaSize)); position+=sizeof(oaSize);
        if (oaSize > 0) {
            memcpy(strBuff, position, oaSize); strBuff[oaSize] = '\0';
            sms.originatingAddress = Address(strBuff);
            position+=oaSize;
        } else throw Exception("OA invalid, size=%d", oaSize);
        memcpy(&daSize, position, sizeof(daSize)); position+=sizeof(daSize);
        if (daSize > 0) {
            memcpy(strBuff, position, daSize); strBuff[daSize] = '\0';
            sms.destinationAddress = Address(strBuff);
            position+=daSize;
        } else throw Exception("DA invalid, size=%d", daSize);
        memcpy(&ddaSize, position, sizeof(ddaSize)); position+=sizeof(ddaSize);
        if (ddaSize > 0) {
            memcpy(strBuff, position, ddaSize); strBuff[ddaSize] = '\0';
            sms.dealiasedDestinationAddress = Address(strBuff);
            position+=ddaSize;
        } else throw Exception("DDA invalid, size=%d", ddaSize);

        memcpy(&sms.messageReference, position, sizeof(sms.messageReference));
        position+=sizeof(sms.messageReference);
        sms.messageReference = ntohs(sms.messageReference);
        
        memcpy(&svcSize, position, sizeof(svcSize)); position+=sizeof(svcSize);
        if (svcSize > 0) {
            if (svcSize <= sizeof(sms.eServiceType)) {
                memcpy(strBuff, position, svcSize); strBuff[svcSize] = '\0';
                sms.setEServiceType(strBuff);
                position+=svcSize;

            } else throw Exception("svcType invalid, size=%d", svcSize);
        } 
        
        memcpy(&sms.deliveryReport, position, sizeof(sms.deliveryReport));
        position+=sizeof(sms.deliveryReport);
        memcpy(&sms.billingRecord, position, sizeof(sms.billingRecord));
        position+=sizeof(sms.billingRecord);
    
        memcpy(&odMscSize, position, sizeof(odMscSize)); position+=sizeof(odMscSize);
        if (odMscSize > 0) {
            memcpy(strBuff, position, odMscSize); strBuff[odMscSize] = '\0';
            sms.originatingDescriptor.setMsc(odMscSize, strBuff);
            position+=odMscSize;
        } else sms.originatingDescriptor.setMsc(0, "");
        memcpy(&odImsiSize, position, sizeof(odImsiSize)); position+=sizeof(odImsiSize);
        if (odImsiSize > 0) {
            memcpy(strBuff, position, odImsiSize); strBuff[odImsiSize] = '\0';
            sms.originatingDescriptor.setImsi(odImsiSize, strBuff);
            position+=odImsiSize;
        } else sms.originatingDescriptor.setImsi(0, "");
        memcpy(&sms.originatingDescriptor.sme, position, sizeof(sms.originatingDescriptor.sme));
        position+=sizeof(sms.originatingDescriptor.sme);
        sms.originatingDescriptor.sme = ntohl(sms.originatingDescriptor.sme);
    
        memcpy(&ddMscSize, position, sizeof(ddMscSize)); position+=sizeof(ddMscSize);
        if (ddMscSize > 0) {
            memcpy(strBuff, position, ddMscSize); strBuff[ddMscSize] = '\0';
            sms.destinationDescriptor.setMsc(ddMscSize, strBuff);
            position+=ddMscSize;
        } else sms.destinationDescriptor.setMsc(0, "");
        memcpy(&ddImsiSize, position, sizeof(ddImsiSize)); position+=sizeof(ddImsiSize);
        if (ddImsiSize > 0) {
            memcpy(strBuff, position, ddImsiSize); strBuff[ddImsiSize] = '\0';
            sms.destinationDescriptor.setImsi(ddImsiSize, strBuff);
            position+=ddImsiSize;
        } else sms.destinationDescriptor.setImsi(0, "");
        memcpy(&sms.destinationDescriptor.sme, position, sizeof(sms.destinationDescriptor.sme));
        position+=sizeof(sms.destinationDescriptor.sme);
        sms.destinationDescriptor.sme = ntohl(sms.destinationDescriptor.sme);
        
        memcpy(&routeSize, position, sizeof(routeSize)); position+=sizeof(routeSize);
        if (routeSize > 0) {
            memcpy(strBuff, position, routeSize); strBuff[routeSize] = '\0';
            sms.setRouteId(strBuff);
            position+=routeSize;
        } else sms.setRouteId("");
        
        memcpy(&sms.serviceId, position, sizeof(sms.serviceId)); position+=sizeof(sms.serviceId);
        sms.serviceId = (int32_t)ntohl(sms.serviceId);
        memcpy(&sms.priority, position, sizeof(sms.priority)); position+=sizeof(sms.priority);
        sms.priority = (int32_t)ntohl(sms.priority);

        memcpy(&srcSmeSize, position, sizeof(srcSmeSize)); position+=sizeof(srcSmeSize);
        if (srcSmeSize > 0) {
            memcpy(strBuff, position, srcSmeSize); strBuff[srcSmeSize] = '\0';
            sms.setSourceSmeId(strBuff);
            position+=srcSmeSize;
        } else sms.setSourceSmeId("");
        memcpy(&dstSmeSize, position, sizeof(dstSmeSize)); position+=sizeof(dstSmeSize);
        if (dstSmeSize > 0) {
            memcpy(strBuff, position, dstSmeSize); strBuff[dstSmeSize] = '\0';
            sms.setDestinationSmeId(strBuff);
            position+=dstSmeSize;
        } else sms.setDestinationSmeId("");

        memcpy(&bodyBufferLen, position, sizeof(bodyBufferLen)); position+=sizeof(bodyBufferLen);
        bodyBufferLen = (int32_t)ntohl(bodyBufferLen);
        if (bodyBufferLen > 0) {
            uint8_t* bodyBuffer = new uint8_t[bodyBufferLen];
            memcpy(bodyBuffer, position, bodyBufferLen); position+=bodyBufferLen;
            sms.messageBody.setBuffer(bodyBuffer, bodyBufferLen);
        } else sms.messageBody.setBuffer(0, 0);
        

    } catch (Exception& exc) {
      Exception e("Inconsistent archive file data. Details: %s", exc.what());
      throw StorageException(e.what());
    } catch (std::exception& exc) {
      Exception e("Inconsistent archive file data. Details: %s", exc.what());
      throw StorageException(e.what());
    } catch (...) {
      throw StorageException("Inconsistent archive file data. Reason unknown");
    }
    
    return true;
}

void BillingStorage::create()
{ 
    static const char* SMSC_BILLING_HEADER_TEXT = 
        "MSG_ID,RECORD_TYPE,MEDIA_TYPE,BEARER_TYPE,SUBMIT,FINALIZED,STATUS,"
        "SRC_ADDR,SRC_IMSI,SRC_MSC,SRC_SME_ID,DST_ADDR,DST_IMSI,DST_MSC,DST_SME_ID,"
        "DIVERTED_FOR,ROUTE_ID,SERVICE_ID,USER_MSG_REF,DATA_LENGTH\n";
    if (RollingStorage::create(true)) {
        write( SMSC_BILLING_HEADER_TEXT, strlen(SMSC_BILLING_HEADER_TEXT));
        flush();
    }
}
void ArchiveStorage::create()
{ 
    if (RollingStorage::create(false)) {
        write( SMSC_ARCHIVE_HEADER_TEXT , strlen(SMSC_ARCHIVE_HEADER_TEXT));
        uint16_t version = htons(SMSC_ARCHIVE_VERSION_INFO);
        write(&version, sizeof(SMSC_ARCHIVE_VERSION_INFO));
        flush();
    }
}

void ArchiveStorage::createRecord(SMSId id, SMS& sms)
{
    MutexGuard guard(storageFileLock);
    this->create();
    save(id, sms);
}

bool PersistentStorage::create(bool create)
{
    if (storageFile) return false;
    
    std::string fullFilePath = storageLocation+'/'+storageFileName;
    const char* fullFilePathStr = fullFilePath.c_str();
    storageFile = fopen(fullFilePathStr, "r");
    
    bool needFile = true;
    if (storageFile)  { // opened for reading
        if (!create) return true;   
        fclose(storageFile); storageFile = 0;
        needFile = false;
    } 
    else if (!create) {  // not exist for reading
        Exception exc("File '%s' not exists. Details: %s", fullFilePathStr, strerror(errno));
        throw StorageException(exc.what());
    }                   // openning for writing (appending)

    storageFile = fopen(fullFilePathStr, "ab+");
    if (!storageFile) {
        Exception exc("Failed to open file '%s' for writing. Details: %s", 
                      fullFilePathStr, strerror(errno));
        throw StorageException(exc.what());
    }
    if (fseek(storageFile, 0, SEEK_END)) {
        int error = ferror(storageFile);
        Exception exc("Failed to seek EOF. Details: %s", strerror(error));
        fclose(storageFile); storageFile = 0;
        throw StorageException(exc.what());
    }
    return needFile;
}
void PersistentStorage::open(bool read)
{ 
    if (PersistentStorage::create(!read))
    {
        if (read) {
            int  headerTextLen = strlen(SMSC_ARCHIVE_HEADER_TEXT);
            char headerText[128];
            if (FileStorage::read(headerText, headerTextLen)) {
                headerText[headerTextLen] = '\0';
                if (strcmp(headerText, SMSC_ARCHIVE_HEADER_TEXT) == 0) {
                    uint16_t version = 0;
                    if (FileStorage::read(&version, sizeof(version)) && 
                        SMSC_ARCHIVE_VERSION_INFO == ntohs(version)) return;
                } 
            }
            throw StorageException("Invalid archive file header format.");
        }
        else {
            FileStorage::write( SMSC_ARCHIVE_HEADER_TEXT , strlen(SMSC_ARCHIVE_HEADER_TEXT));
            uint16_t version = htons(SMSC_ARCHIVE_VERSION_INFO);
            FileStorage::write(&version, sizeof(SMSC_ARCHIVE_VERSION_INFO));
            FileStorage::flush();
        }
    }
}

void PersistentStorage::writeRecord(SMSId id, SMS& sms, fpos_t* pos /*= 0 (no getPos) */)
{
    MutexGuard guard(storageFileLock);
    this->open(false);
    FileStorage::save(id, sms, pos);
}
bool PersistentStorage::readRecord(SMSId& id, SMS& sms, const fpos_t* pos /*= 0 (no setPos) */)
{
    MutexGuard guard(storageFileLock);
    this->open(true);
    return FileStorage::load(id, sms, pos);
}

}}
