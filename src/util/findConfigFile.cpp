#include "util/findConfigFile.h"
#include <sys/stat.h>
#include "core/buffers/TmpBuf.hpp"

namespace smsc {
namespace util {

using smsc::core::buffers::TmpBuf;

bool file_exist(const char * const filename)
{
  struct stat buf;
  return stat(filename, &buf) == 0;
}

char get_filename_result[128];

const char * const findConfigFile(const char * const file_to_find)
  throw (smsc::admin::AdminException)
{
  if (file_exist(file_to_find))
  {
    strcpy(get_filename_result, file_to_find);
    return get_filename_result;
  }
  else
  {
    //char buf[strlen(file_to_find)+8+1];
    TmpBuf<char,1024> buf(strlen(file_to_find)+8+1);
    strcpy(buf, "../conf/");
    strcat(buf, file_to_find);
    if (file_exist(buf))
    {
      strcpy(get_filename_result, buf);
      return get_filename_result;
    }
    else
    {
      //char buf[strlen(file_to_find)+8+1];
      TmpBuf<char,1024> buf(strlen(file_to_find)+8+1);
      strcpy(buf, "./conf/");
      strcat(buf, file_to_find);
      if (file_exist(buf))
      {
        strcpy(get_filename_result, buf);
        return get_filename_result;
      }
    }
  }
  //char message[strlen(file_to_find) + 64];
  TmpBuf<char,1024> message(strlen(file_to_find) + 64);
  sprintf(message, "File \"%s\" not found", file_to_find);
  throw smsc::admin::AdminException(message);
}

}
}
