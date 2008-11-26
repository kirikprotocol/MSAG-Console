#include "Inst.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "scag/util/lltostr.h"

void Inst::check(const char* fname)
{
    char *p, pid[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    filename = fname;
    
    int fd = open(filename.c_str(), O_RDONLY);
    if(fd != -1)
    {
        read(fd, pid, 19);
        close(fd);
        run_ = !IsProcDir(pid);
        if(!run_) return;        
    }
   
    if((fd = open(filename.c_str(), O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) == -1)
        return;
    p = lltostr(getpid(), pid + 19);
    write(fd, p, pid + 19 - p);
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
