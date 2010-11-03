#ifndef _INFORMER_DIRLISTING_H
#define _INFORMER_DIRLISTING_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <cerrno>
#include "InfosmeException.h"
#include "core/buffers/TmpBuf.hpp"

namespace eyeline {
namespace informer {

struct NoDotsNameFilter 
{
    inline bool operator() ( const char* fn ) const {
        return !(0 == strcmp(fn,".") || 0 == strcmp(fn,".."));
    }
};


template <class NameFilter>
class DirListing
{
public:
    DirListing( const NameFilter& nameFilter, mode_t statFilter = S_IFMT ) :
    nameFilter_(nameFilter), statFilter_(statFilter) {}

    void list( const char* path,
               std::vector< std::string >& contents ) const
    {
        DIR* dir = opendir(path);
        if (!dir) {
            throw ErrnoException(errno,"opendir('%s')",path);
        }
        struct CloseDirGuard {
            CloseDirGuard(DIR* d) : dir(d) {}
            ~CloseDirGuard()
            {
                closedir(dir);
            }
            DIR* dir;
        } guard(dir);
        static const size_t bufsize = 512;
        smsc::core::buffers::TmpBuf<char,bufsize>
            buf(sizeof(dirent) +
                pathconf(const_cast<char*>(path),_PC_NAME_MAX)+1 );
        dirent* de = (dirent*)buf.get();
        dirent* ptr;
        std::string fpath;
        do {
            int err = readdir_r(dir,de,&ptr);
            if (err) {
                throw ErrnoException(errno,"readdir('%s')",path);
            }
            if (!ptr) break;
            do {
                if ( !nameFilter_(ptr->d_name) ) {
                    // filtered
                    break;
                }
                if (statFilter_ != S_IFMT) {
                    struct ::stat st;
                    fpath = path;
                    if (!fpath.empty() && fpath[fpath.size()-1] != '/') fpath += '/';
                    fpath += ptr->d_name;
                    if (-1 == ::stat(fpath.c_str(),&st)) {
                        throw ErrnoException(errno,"stat('%s')",
                                             fpath.c_str());
                    }
                    if ( (st.st_mode & statFilter_) ==0 ) {
                        // filtered
                        break;
                    }
                }
                contents.push_back(ptr->d_name);
            } while (false);
        } while (true);
    }

private:
    NameFilter  nameFilter_;
    mode_t      statFilter_;
};


template <class NameFilter>
    DirListing<NameFilter> makeDirListing( const NameFilter& nameFilter,
                                           mode_t statFilter = S_IFMT )
{
    return DirListing<NameFilter>(nameFilter,statFilter);
}

} // informer
} // smsc

#endif
