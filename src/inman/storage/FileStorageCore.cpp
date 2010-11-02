#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "inman/storage/FileStorageCore.hpp"
#include <core/buffers/TmpBuf.hpp>

using smsc::core::buffers::FileException;
using smsc::core::buffers::TmpBuf;

namespace smsc {
namespace inman {
namespace filestore {

using smsc::core::synchronization::MutexGuard;

/* ************************************************************************** *
 * class FileSystemException implementation:
 * ************************************************************************** */
FileSystemException::FileSystemException(const FileSystemException& exc)
    : Exception(exc.what())
    , _code(exc.getErrorCode()), _status(exc.getErrorStatus())
{
}

FileSystemException::FileSystemException(int exCode, const char* fmt, ...)
    : _code(exCode), _status(0)
{
    SMSC_UTIL_EX_FILL(fmt);
}

FileSystemException::FileSystemException(const char* cause, int exCode, int status)
    : Exception(cause)
    , _code(exCode), _status(status)
{
}

/* ************************************************************************** *
 * class FSEntry implementation:
 * ************************************************************************** */
bool FSEntry::createDir(const std::string& dir, mode_t mode)
{
    if (mkdir(dir.c_str(), mode)) {
        if (errno == EEXIST)
            return false;
        throw FileSystemException(errno, "Failed to create directory '%s'. Details: %s",
                                    dir.c_str(), strerror(errno));
    }
    return true;
}

bool FSEntry::createDir(const std::string& dir)
{
    return createDir(dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
}

void FSEntry::findEntries(const std::string& location, FSEntriesArray& entries,
                              bool files, const char* ext)
{
    size_t extFileLen  = 0;
    const char* locationStr = location.c_str();

    if (files) {
        if (!ext || !ext[0])
            return;
        extFileLen = strlen(ext)+1;
    }

    DIR *locationDir = 0;
    if (!(locationDir = opendir(locationStr))) {
        throw FileSystemException(errno, "Failed to open directory '%s'. Details: %s",
                                   locationStr, strerror(errno));
    }

    TmpBuf<char, 1024> entryGuard(sizeof(struct dirent) + pathconf(locationStr, _PC_NAME_MAX));
    char* entry = entryGuard.get();
    struct dirent* pentry = 0;

    while (locationDir) {
        errno = 0;
        int result = readdir_r(locationDir, (struct dirent *)entry, &pentry);
        if (!result && pentry) {
            struct stat     description;
            std::string     entryName = location;
            entryName += '/';
            entryName += pentry->d_name;
            if (stat(entryName.c_str(), &description) != 0) {
                if (locationDir)
                    closedir(locationDir);
                throw FileSystemException(errno, "Failed to obtain '%s' entry info. Details: %s",
                                        entryName.c_str(), strerror(errno));
            }
            if (files) {
                if (!(description.st_mode & S_IFDIR)) {
                    size_t fileNameLen = strlen(pentry->d_name);
                    if (fileNameLen > extFileLen) {
                        const char* extPos = pentry->d_name + (fileNameLen - extFileLen);
                        if ((*extPos == '.') && !strcmp(extPos + 1, ext))
                            entries.Push(pentry->d_name);
                    }
                }
            } else if (description.st_mode & S_IFDIR) {
                if (strcmp(pentry->d_name, ".") && strcmp(pentry->d_name, ".."))
                    entries.Push(pentry->d_name);
            }
        } else {
            if (!errno)
                break;
            if (locationDir)
                closedir(locationDir);
            throw FileSystemException(errno, "Failed to scan directory '%s' contents. Details: %s",
                          locationStr, strerror(errno));
        }
    }
    if (locationDir)
        closedir(locationDir);
}

void FSEntry::findFiles(const std::string& location, const char* ext, FSEntriesArray& files)
{
    FSEntry::findEntries(location, files, true, ext);
}
void FSEntry::findDirs (const std::string& location, FSEntriesArray& dirs)
{
    FSEntry::findEntries(location, dirs, false, 0);
}

void FSEntry::deleteFile(const std::string& fullPath)
{
    try { File::Unlink(fullPath.c_str()); }
    catch ( FileException& exc) {
        throw FileSystemException(exc.getErrNo(), exc.what());
    }
}
void FSEntry::deleteFile(const std::string& location, const std::string& fileName)
{
    std::string fullPath = location;
    fullPath +='/';
    fullPath += fileName;
    deleteFile(fullPath);
}

void FSEntry::truncateFile(const std::string& fullPath, off_t length)
{
    if (truncate(fullPath.c_str(), length) != 0) {
        throw FileSystemException(errno, "Failed to truncate file '%s' to %d bytes. Details: %s",
                      fullPath.c_str(), length, strerror(errno));
    }
}
void FSEntry::truncateFile(const std::string& location, const std::string& fileName, off_t length)
{
    std::string fullPath = location;
    fullPath += '/';
    fullPath += fileName;
    truncateFile(fullPath, length);
}

File::offset_type FSEntry::getFilePos(File & currFile)
{
    File::offset_type    pos;

    try { pos = currFile.Pos(); }
    catch (FileException& exc) {
        try { currFile.Close(); } catch (...) {}
        throw FileSystemException(exc.getErrNo(), exc.what());
    }
    return pos;
}
//NOTE: NULL as argument adjusts position to start of file
void FSEntry::setFilePos(File & currFile, const File::offset_type* pos)
{
    File::offset_type newPos = pos ? *pos : 0;

    try { currFile.Seek(newPos, SEEK_SET); }
    catch (FileException& exc) {
        try { currFile.Close(); } catch (...) {}
        throw FileSystemException(exc.getErrNo(), exc.what());
    }
}

void FSEntry::rollFileExt(std::string& fullPath, const char* newExt)
{
    if (!newExt || !newExt[0])
        return;

    std::string             fullOldFile = fullPath;
    std::string::size_type  extpos = fullPath.find_last_of('.');
    if (extpos != fullPath.npos)
        fullPath.erase(extpos, fullPath.npos);
    fullPath += '.';
    fullPath += newExt;

    try { File::Rename(fullOldFile.c_str(), fullPath.c_str()); }
    catch (FileException& exc) {
        throw FileSystemException(exc.getErrNo(), exc.what());
    }
}

void FSEntry::rollFileExt(const std::string& location, std::string& fileName, const char* newExt)
{
    std::string     fullOldFile = location;
    fullOldFile += '/';
    fullOldFile += fileName;
    rollFileExt(fullOldFile, newExt);
}

void FSEntry::rollFileEnding(std::string& fullPath, const char* newExt, const char * oldExt)
{
    if (!newExt || !newExt[0] || !oldExt ||!oldExt[0])
        return;

    std::string     fullOldFile = fullPath;
    std::string::size_type fileNameLen = fullPath.length();
    size_t oldExtLen = strlen(oldExt);
    fullPath.erase(fileNameLen - oldExtLen, fullPath.npos);
//    fullPath += '.';
    fullPath += newExt;

    try { File::Rename(fullOldFile.c_str(), fullPath.c_str()); }
    catch (FileException& exc) {
        throw FileSystemException(exc.getErrNo(), exc.what());
    }
    return;
}

/* ************************************************************************** *
 * class RollingFileStorage implementation:
 * ************************************************************************** */
static const char _fmtTStamp[] = "%Y%m%d_%H%M%S";

RollingFileStorage::RollingFileStorage(const std::string & location, const char *lastExt,
                                    const char *storageExt, unsigned long rollInterval,
                                    const RollingFileStorageParms * parms /* = NULL*/)
    : _location(location), _headerLen(0), _lastRollTime(0), _interval(rollInterval)
{
    _lastExt += lastExt;
    _Ext += storageExt;
    _Parms = *parms;
    if (!_Parms.timeStampFormat)
        _Parms.timeStampFormat = &_fmtTStamp[0];
    if (_Parms.fileHeaderText)
        _headerLen = strlen(_Parms.fileHeaderText);
    if (_Parms.printHeaderVersion) {
        _Parms.headerVersion = htons(_Parms.headerVersion);
        _headerLen += sizeof(_Parms.headerVersion);
    }
}
RollingFileStorage::~RollingFileStorage()
{
    if (_currFile.isOpened()) { //if file is empty -> delete it
        File::offset_type    pos;

        try { pos = _currFile.Pos(); }
        catch (...) { pos = _headerLen + 1; }

        try { _currFile.Close(); }
        catch (...) {}

        if (pos <= _headerLen) {// file is empty
            try { _currFile.Unlink((_currFile.getFileName()).c_str()); }
            catch (...) {}
        }
    }
}

bool RollingFileStorage::FSOpen(bool rollOld/* = true*/)
{
    MutexGuard guard(_storageLock);
    //check for location and old storage files
    bool oldStore = !FSEntry::createDir(_location);

    if (oldStore) { //find old storage files
        FSEntriesArray  files;
        FSEntry::findFiles(_location, _lastExt.c_str(), files);

        for (int i = 0; rollOld && (i < files.Count()); i++) {
            //FSEntry::rollFileExt(_location, files[i], _Ext.c_str());
            std::string fullName(_location);
            fullName += '/';
            fullName += files[i];
            FSEntry::rollFileEnding(fullName, _Ext.c_str(), _lastExt.c_str());
        }


        if (!rollOld) { //delete all old files
            FSEntry::findFiles(_location, _Ext.c_str(), files);
            for (int i = 0; i < files.Count(); i++)
                FSEntry::deleteFile(_location, files[i]);
        }
    }
    //create current file
    mkCurrFile(false);
    return oldStore;
}

bool RollingFileStorage::FSRoll(void)
{
    MutexGuard guard(_storageLock);
    return mkCurrFile(true);
}

void RollingFileStorage::FSFlush(void)
{
    MutexGuard guard(_storageLock);
    try { _currFile.Flush(); }
    catch (FileException& exc) {
        try { _currFile.Close(); } catch (...) {}
        throw FileSystemException(exc.getErrNo(), exc.what());
    }
}

void RollingFileStorage::FSClose(void)
{
    MutexGuard guard(_storageLock);
    //first check if last file is empty. Delete it in that case.
    if (_currFile.isOpened()) {
        File::offset_type    pos;

        try { pos = _currFile.Pos(); }
        catch (FileException& exc) {
            throw FileSystemException(exc.getErrNo(), exc.what());
        }
        if (pos <= _headerLen) {// file is empty -> delete it
            try { _currFile.Close(); }
            catch (FileException& exc) {
                throw FileSystemException(exc.getErrNo(), exc.what());
            }
            try { _currFile.Unlink((_currFile.getFileName()).c_str()); }
            catch (FileException& exc) {
                throw FileSystemException(exc.getErrNo(), exc.what());
            }
        } else { //file contains data, flush and close it
            try { _currFile.Close(); }
            catch (FileException& exc) {
                throw FileSystemException(exc.getErrNo(), exc.what());
            }
        }
    }
}

bool RollingFileStorage::FSWrite(const void* data, size_t size, bool roll/* = true*/)
{
    MutexGuard guard(_storageLock);
    bool rolled = false;
    if (roll && _interval) {
        time_t  curTm = time(NULL);
        if (curTm >= (_lastRollTime + _interval))
            rolled = mkCurrFile(true);
    }
    try { _currFile.Write(data, size); }
    catch (FileException& exc) {
        throw FileSystemException(exc.getErrNo(), exc.what());
    }
    return rolled;
}

int  RollingFileStorage::getStorageFiles(FSEntriesArray& files)
{
    FSEntry::findFiles(_location, _lastExt.c_str(), files);
    FSEntry::findFiles(_location, _Ext.c_str(), files);
    return files.Count();
}

time_t RollingFileStorage::getLastRollingTime(void) const
{
    return _lastRollTime;
}
/* -------------------------------------------------------------------------- *
 * PROTECTED:
 * -------------------------------------------------------------------------- */
bool RollingFileStorage::mkCurrFile(bool roll/* = true*/)
{
    if (_currFile.isOpened() && !roll) //file already exists, exit
        return false;

    time_t      curTm = time(NULL);
    char        tmStamp[MAX_FS_TIME_STAMP_SZ + 1];
    //NOTE:  cftime allows [00,61] range for seconds, instead of [00,59],
    //so use gmtime_r() that adjusts seconds first and then strftime, that
    //also checks for ABW (format may not be default!)
    struct tm   dtm;
    gmtime_r(&curTm, &dtm);
    size_t  tmLen = strftime(tmStamp, MAX_FS_TIME_STAMP_SZ, _Parms.timeStampFormat, &dtm);
    if (!tmLen || (tmLen >= MAX_FS_TIME_STAMP_SZ))
        throw FileSystemException(-2, "User time stamp format is invalid or too long!");

    std::string filePath = _location;
    filePath += '/';
    filePath += tmStamp;
    filePath += '.';

    _lastRollTime = curTm;

    if (_currFile.isOpened()) {
        File::offset_type fpos;

        try { fpos = _currFile.Pos(); }
        catch (FileException& exc) {
            throw FileSystemException(exc.getErrNo(), exc.what());
        }
        if (fpos <= _headerLen) {// file is empty, skip rolling but rename file
            std::string rollName = filePath;
            rollName += _lastExt;

            try { _currFile.Rename(rollName.c_str()); }
            catch (FileException& exc) {
                throw FileSystemException(exc.getErrNo(), exc.what());
            }
            return false;
        }
        //roll and close file
        try { _currFile.Close(); }
        catch (FileException& exc) {
            throw FileSystemException(exc.getErrNo(), exc.what());
        }
//        FSEntry::rollFileExt((std::string&)_currFile.getFileName(), _Ext.c_str());
        FSEntry::rollFileEnding((std::string&)_currFile.getFileName(), _Ext.c_str(), _lastExt.c_str());
    }

    filePath += _lastExt;

    if (File::Exists(filePath.c_str()))
        throw FileSystemException(EEXIST, "Failed to create new file '%s'. File already exists!",
                                   filePath.c_str());

    try { _currFile.RWCreate(filePath.c_str()); }
    catch (FileException& exc) {
        throw FileSystemException(exc.getErrNo(), "Failed to create new file '%s'. Details: %s",
                                   filePath.c_str(), exc.what());
    }

    //write header
    if (_Parms.fileHeaderText) {
        try { _currFile.Write(_Parms.fileHeaderText, strlen(_Parms.fileHeaderText)); }
        catch (FileException& exc) {
            throw FileSystemException(exc.getErrNo(), exc.what());
        }
    }
    if (_Parms.printHeaderVersion) {
        try { _currFile.Write(&_Parms.headerVersion, sizeof(_Parms.headerVersion)); }
        catch (FileException& exc) {
            throw FileSystemException(exc.getErrNo(), exc.what());
        }
    }

    return true;
}

} //filestore
} //inman
} //smsc

