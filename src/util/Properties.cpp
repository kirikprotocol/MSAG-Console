#include "util/Properties.h"

#include <stdio.h>
#include "util/cstrings.h"
#include "util/findConfigFile.h"

namespace smsc {
namespace util {

Properties::Properties()
{}

Properties::Properties(const char * const filename) throw (Exception)
{
  const char * const fn = findConfigFile(filename);

  char buffer[1024];
  FILE * f = fopen(fn, "r");
  if (f != NULL) {
    for (char * str = (char*)""; str != NULL; str = fgets(buffer, sizeof(buffer)/sizeof(buffer[0]), f)) {
      if (str[0] != 0 && str[0] != '#') {
        char * const pos = strchr(str, '=');
        if (pos != NULL) {
          *pos = 0;
          Insert(trim(str), cStringCopy(trim(pos+1)));
        } else {
          Insert(trim(str), cStringCopy(""));
        }
      }
    }
  }
  fclose(f);
}

Properties::~Properties()
{
  char **keys = new char*[GetCount()];
  char * key;
  PropertiesValue val;
  int count=0;
  for (Iterator i = getIterator(); i.Next(key, val); ) {
    keys[count++] = key;
  }

  for (int i = 0; i < count; i++)
  {
    PropertiesValue val = this->Get(keys[i]);
    Delete(keys[i]);
    delete [] val;
  }
  delete [] keys;
}

std::auto_ptr<Properties> Properties::getSection(const char * const prefix) const
{
  const size_t prefixLength = strlen(prefix);
  std::auto_ptr<Properties> result(new Properties());

  char * key;
  PropertiesValue val;
  for (Iterator i = getIterator(); i.Next(key, val); ) {
    if (strlen(key) > prefixLength && startsWith(key, prefix) && key[prefixLength] == '.') {
      result->Insert(key+prefixLength+1, cStringCopy(val));
    }
  }
  return result;
}

}
}
