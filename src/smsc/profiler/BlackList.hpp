#ifndef __SMSC_PROFILER_BLACKLIST_HPP__
#define __SMSC_PROFILER_BLACKLIST_HPP__

#include <set>
#include <string>

#include "core/synchronization/Mutex.hpp"
#include "core/buffers/File.hpp"
#include <sys/types.h>
#include <sys/stat.h>


namespace smsc{
namespace profiler{

class BlackList{
public:
  BlackList(time_t ci):checkInterval(ci)
  {
  }
  void Load(const char* file=0)
  {
    if(file)
    {
      fileName=file;
    }
    smsc::core::buffers::File f;
    f.ROpen(fileName.c_str());
    std::string s;
    lst.clear();
    while(f.ReadLine(s))
    {
      lst.insert(s);
    }
    f.Close();
    struct stat st;
    stat(fileName.c_str(),&st);
    mtime=st.st_mtime;
    lastCheck=time(NULL);
  }
  bool check(const std::string& word)
  {
    time_t now=time(NULL);
    if(now-lastCheck>checkInterval)
    {
      smsc::core::synchronization::MutexGuard mg(mtx);
      if(now-lastCheck>checkInterval)
      {
        Load();
      }
    }
    smsc::core::synchronization::MutexGuard mg(mtx);
    return lst.find(word)!=lst.end();
  }
protected:
  std::set<std::string> lst;
  smsc::core::synchronization::Mutex mtx;
  std::string fileName;
  time_t mtime;
  time_t lastCheck;
  time_t checkInterval;
};

}
}

#endif

