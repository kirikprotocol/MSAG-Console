#include "Inst.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include "lltostr.h"

namespace scag {
namespace util {

void Inst::check(const char* fname)
{
    char *p;
    memset(oldpid_,'\0',sizeof(oldpid_));
    
    filename = fname;
    
    int fd = open(filename.c_str(), O_RDONLY);
    if(fd != -1)
    {
        read(fd, oldpid_, 19);
        close(fd);
        run_ = !IsProcDir(oldpid_);
        if(!run_) return;        
    }
   
    if((fd = open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) == -1)
        return;
    snprintf(oldpid_,sizeof(oldpid_),"%lld",static_cast<long long>(getpid()));
    write(fd, oldpid_, strlen(oldpid_));
    close(fd);
}

bool Inst::IsProcDir(char* oldpid)
{
    DIR* dirp = opendir("/proc");
    dirent *dp;
    
    if(!dirp) return false;
    
    while((dp = readdir(dirp)) && ::strcmp(dp->d_name, oldpid));
    
    closedir(dirp);    
    
    return dp != NULL;
}

} // namespace util
} // namespace scag
