#ifndef __RESOURCE_MANAGER_HPP__
#define __RESOURCE_MANAGER_HPP__

#include <string>
#include <map>
#include <resourcemanager/LocaleResources.hpp>
#include <util/Exception.hpp>

using smsc::util::Exception;

namespace smsc{
namespace resourcemanager{

class ResourceManager{
public:
	~ResourceManager();

  // возвращает строку из сетингов для определленой локали и ключа.
  std::string getSetting(const std::string& locale,const std::string& key);
  // возвращает строку из сетингов для дефолтной локали и ключа.
  std::string getSetting(const std::string& key );
  // возвращает строку из ресурса для определленой локали и ключа.
  std::string getString(const std::string& locale, const std::string& key);
  // возвращает строку из ресурса для дефолтной локали и ключа.
  std::string getString(const std::string& key);

  static ResourceManager& getInstance();

	#ifdef SMSC_DEBUG
	void dump(std::ostream & outStream);
	#endif //#ifdef SMSC_DEBUG

private:
	static std::auto_ptr<ResourceManager> instance;
	static const std::string defaultLocale;
	typedef std::map<std::string, LocaleResources*> _LocalesMap;
	_LocalesMap locales;

	ResourceManager() throw (Exception);
};

};//resourcemanager
};//smsc

#endif
