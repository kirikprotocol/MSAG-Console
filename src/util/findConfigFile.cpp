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

char get_filename_result[1024];
const size_t filename_result_maxlen = sizeof(get_filename_result)/sizeof(get_filename_result[0]);

const char * const findConfigFile(const char * const file_to_find)
  throw (Exception)
{
  if (file_exist(file_to_find))
  {
    if (strlen(file_to_find)+1 > filename_result_maxlen)
      throw smsc::util::Exception("Internal error in findConfigFile: insufficient internal buffer for result filename");
    strcpy(get_filename_result, file_to_find);
    return get_filename_result;
  }
  else
  {
    TmpBuf<char,1024> buf(strlen(file_to_find)+8+1);
    strcpy(buf, "./conf/");
    strcat(buf, file_to_find);
    if (file_exist(buf))
    {
      strcpy(get_filename_result, buf);
      return get_filename_result;
    }
    else
    {
      TmpBuf<char,1024> buf2(strlen(file_to_find)+8+1);
      strcpy(buf2, "../conf/");
      strcat(buf2, file_to_find);
      if (file_exist(buf2))
      {
        strcpy(get_filename_result, buf2);
        return get_filename_result;
      }
    }
  }
  throw smsc::util::Exception("File \"%s\" not found", file_to_find);
}

}
}
