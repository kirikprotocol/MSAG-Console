#ifndef __RESOURCE_MANAGER_HPP__
#define __RESOURCE_MANAGER_HPP__

#include <string>

namespace smsc{
namespace resourcemanager{

class ResourceManager{
public:
  // возвращает строку из сетингов для определленой локали и ключа.
  string getSetting(const string& locale,const string& key);
  // возвращает строку из сетингов для дефолтной локали и ключа.
  string getSetting(const string& key );
  // возвращает строку из ресурса для определленой локали и ключа.
  string getString(const string& locale, const string& key);
  // возвращает строку из ресурса для дефолтной локали и ключа.
  string getString(const string& key);

  static ResourceManager& getInstance();
};

};//resourcemanager
};//smsc

#endif
