#ifndef _FILE_STORAGE_CORE_H
#ident "@(#)$Id$"
#define _FILE_STORAGE_CORE_H

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <string>

#include "core/buffers/Array.hpp"
#include "core/buffers/File.hpp"
#include "core/synchronization/Mutex.hpp"
#include "util/Exception.hpp"

using smsc::core::synchronization::Mutex;
using smsc::core::buffers::File;
using smsc::util::Exception;

namespace smsc {
namespace inman {
namespace filestore {

class FileSystemException : public Exception {
protected:
    int   _code;   //exception code
    int   _status; //thrower status

public:
    FileSystemException(const FileSystemException& exc);
    FileSystemException(int exCode /* = errno*/, const char* fmt, ...);
    FileSystemException(const char* cause, int exCode = -1, int status = 0);
    virtual ~FileSystemException() throw()
    { }

    int getErrorCode() const { return _code; }
    int getErrorStatus() const { return _status; }
};


typedef smsc::core::buffers::Array<std::string> FSEntriesArray;

//FileSystemEntry: general file/directory operations:
struct FSEntry { //all methods throws FileSystemException
public:
    
//directory operations:
    static bool createDir(const std::string& dir); //mode: 775
    static bool createDir(const std::string& dir, mode_t mode);
    static void findEntries(const std::string& location, FSEntriesArray& entries,
                            bool files = false, const char* ext = 0);
    static void findDirs(const std::string& location, FSEntriesArray& dirs);
    static void findFiles(const std::string& location, const char* ext, FSEntriesArray& files);

//file operations:
    static void deleteFile(const std::string& fullPath);
    static void deleteFile(const std::string& location, const std::string& fileName);
    static void truncateFile(const std::string& fullPath, off_t length);
    static void truncateFile(const std::string& location, const std::string& fileName, off_t length);
    static File::offset_type getFilePos(File & currFile);
    //NOTE: NULL as argument adjusts position to start of file
    static void setFilePos(File & currFile, const File::offset_type* pos);

    //upon return fullName keeps file name with new extension
    static void rollFileExt(const std::string& location, std::string& fullName, const char* newExt);
    static void rollFileExt(std::string& fullPath, const char* newExt);
    //relaces ending part of name (concatenated extensions)
    static void rollFileEnding(std::string& fullPath, const char* newExt, const char * oldExt);
};

//Additional RollingFileStorage configuration parameters
#define MAX_FS_TIME_STAMP_SZ 256
typedef struct {
    const char* fileHeaderText;     //start file with header text
    bool        printHeaderVersion; //add header version to header text
    uint16_t    headerVersion;      //just a number
    const char* timeStampFormat;    //strftime() time stamp format, default "%Y%m%d_%H%M%S"
                                    //that results in "YYYYMMDD_hhmmss" being printed
} RollingFileStorageParms;


class RollingFileStorage { //all methods throws FileSystemException
protected:
    File            _currFile;
    Mutex           _storageLock;
    std::string     _location;
    std::string     _lastExt;
    std::string     _Ext;
    RollingFileStorageParms _Parms;
    size_t          _headerLen;
    time_t          _lastRollTime;
    long            _interval;
    
public:
    //'rollInterval' == 0 forbids rolling by timer
    RollingFileStorage(const std::string & location, const char *lastExt,
                       const char *storageExt, unsigned long rollInterval, //in seconds
                       const RollingFileStorageParms * parms = NULL);
    virtual ~RollingFileStorage();

    //Opens rolling file storage, able to init storage over existing one,
    //old files are rolled by default
    //NOTE: if rollOld == false, all old files will be deleted
    //Returns true if old storage detected
    bool FSOpen(bool rollOld = true);
    //roll file storage
    bool FSRoll(void);
    //returns true if storage was rolled (by time interval)
    //setting 'roll' = false forbids rolling at all
    bool FSWrite(const void* data, size_t size, bool roll = true);
    void FSFlush(void);
    //
    void FSClose(void);
    //returns number of files in storage, appends to 'files' their names
    int  getStorageFiles(FSEntriesArray& files);
    time_t getLastRollingTime(void) const;

protected:
    //creates and initializes current storage file, returns true on rolling
    bool mkCurrFile(bool roll = true);
};


} //filestore
} //inman
} //smsc

#endif /* _FILE_STORAGE_CORE_H */

