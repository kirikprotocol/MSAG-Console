#include "logger/additional/RollingFileAppender.h"

#include <time.h>
#include <sys/time.h>
#ifndef _WIN32
#include <pthread.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#include "util/cstrings.h"
#include "core/buffers/TmpBuf.hpp"

namespace smsc {
namespace logger {

using namespace smsc::util;
using namespace smsc::core::buffers;

void rolloverFiles(const char * const filename, unsigned int maxBackupIndex)
{
  const size_t max_filename_length = strlen(filename)+16;
  std::auto_ptr<char> buffer(new char[max_filename_length+1]);
  snprintf(buffer.get(), max_filename_length, "%s.%u", filename, maxBackupIndex);
  buffer.get()[max_filename_length] = 0;
  if(File::Exists(buffer.get()))File::Unlink(buffer.get());

  // Map {(maxBackupIndex - 1), ..., 2, 1} to {maxBackupIndex, ..., 3, 2}
  for(int i=maxBackupIndex - 1; i >= 1; i--) {
    std::auto_ptr<char> source(new char[max_filename_length+1]);
    std::auto_ptr<char> target(new char[max_filename_length+1]);

    snprintf(source.get(), max_filename_length, "%s.%u", filename, i);
    snprintf(target.get(), max_filename_length, "%s.%u", filename, i+1);

    source.get()[max_filename_length] = 0;
    target.get()[max_filename_length] = 0;

    if(File::Exists(source.get()))File::Rename(source.get(), target.get());
  }
}


RollingFileAppender::RollingFileAppender(const char * const _name, const Properties & properties)
  :Appender(_name), mutex(MTXFORCENOCHECK)
{
  try{
    maxFileSize = 1*1024*1024;
    maxBackupIndex = 5;
    directIO=false;

    if(properties.Exists("maxsize")) {
      const char * const tmp = properties["maxsize"];
      const size_t tmpLen = strlen(tmp);
      maxFileSize = atoi(tmp);
      if (tmpLen > 2)
      {
        const char * const suffix = tmp + (tmpLen -2);
        if (strcasecmp("mb", suffix) == 0)
        {
          maxFileSize *= (1024 * 1024); // convert to megabytes
        }
        else if (strcasecmp("kb", suffix) == 0)
        {
          maxFileSize *= 1024; // convert to kilobytes
        }
      }
    }

    if(properties.Exists("directIO"))
    {
      directIO=strcasecmp(properties["directIO"],"true")==0;
    }

    if (properties.Exists("maxindex"))
      maxBackupIndex = atoi(properties["maxindex"]);

    if (properties.Exists("name"))
      filename.reset(cStringCopy(properties["name"]));
    else
      filename.reset(cStringCopy("smsc.log"));

    if(File::Exists(filename.get()))
      file.Append(filename.get());
    else
      file.WOpen(filename.get());
    currentFilePos=file.Pos();
    if(directIO)
    {
      file.EnableDirectIO();
    }
    //file = fopen(filename.get(), "a");
    //currentFilePos = ftell(file);
  }catch(std::exception& e)
  {
    fprintf(stderr,"RollingFileAppender exception:%s\n",e.what());
  }
}

void RollingFileAppender::rollover() throw()
{
  // If maxBackups <= 0, then there is no file renaming to be done.
  try{
    if(maxBackupIndex > 0) {
      rolloverFiles(filename.get(), maxBackupIndex);

      // Close the current file
      //fclose(file);
      //file = NULL;
      file.Close();

      // Rename fileName to fileName.1
      const size_t max_filename_length = strlen(filename.get())+16;
      std::auto_ptr<char> target(new char[max_filename_length+1]);
      snprintf(target.get(), max_filename_length, "%s.1", filename.get());
      target.get()[max_filename_length] = 0;

      File::Rename(filename.get(), target.get());

      // Open a new file
      //file = fopen(filename.get(), "w");
      file.WOpen(filename.get());
      if(directIO)
      {
        file.EnableDirectIO();
      }
    } else {
      //fclose(file);
      //file = fopen(filename.get(), "w");
      file.Close();
      file.WOpen(filename.get());
      if(directIO)
      {
        file.EnableDirectIO();
      }
    }
  }catch(std::exception& e)
  {
    fprintf(stderr,"RollingFileAppender::rollover exception:%s\n",e.what());
  }catch(...)
  {
    fprintf(stderr,"RollingFileAppender::rollover unknown exception\n");
  }

  currentFilePos = 0;
}

void RollingFileAppender::log(timeval tp,const char logLevelName, const char * const category, const char * const message) throw()
{
  //D dd-mm hh:mm:ss,sss TTT CatLast___:message
  ::tm lcltm;
  ::localtime_r(&tp.tv_sec, &lcltm);
  pthread_t thrId=::pthread_self();
  long msec=tp.tv_usec/1000;
  char timeStr[32];
  const size_t timeStrLength = ::strftime(timeStr, sizeof(timeStr)/sizeof(timeStr[0]), "%d-%m %H:%M:%S", &lcltm);
  timeStr[timeStrLength] = 0;
  const size_t desiredLength = strlen(message)+128;
  TmpBuf<char, 4096> buffer(desiredLength+1);
  const size_t length = snprintf(buffer, desiredLength, "%c %s,%3.3u %3.3u %10.10s: %s\n", logLevelName, timeStr, unsigned(msec), unsigned(thrId), category, message);
  buffer[desiredLength] = 0;

  try
  {
    {
      smsc::core::synchronization::MutexGuard guard(mutex);
      if(file.isOpened())
      {
        file.Write(buffer, length < desiredLength ? length : desiredLength);
        file.Flush();
      }else
      {
        fwrite(buffer, length < desiredLength ? length : desiredLength, 1, stderr);
      }
      //fflush(file);
      currentFilePos += length;
      if (currentFilePos > maxFileSize)
        rollover();
    }

  } catch(std::exception& e)
  {
    fprintf(stderr,"EXCEPTION IN LOGGER:%s\n",e.what());
  }
}

}
}
