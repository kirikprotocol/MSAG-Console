#ifndef __LOCALE_RESOURCES_HPP__
#define __LOCALE_RESOURCES_HPP__

#include <string>
#include <map>
#include <xercesc/dom/DOM_Element.hpp>
#include "util/templates/Formatters.h"

namespace smsc{
namespace resourcemanager{

using namespace smsc::util::templates;

class LocaleResources{
public:
  LocaleResources(const std::string &filename) throw ();
  ~LocaleResources();

  // возвращает строку из сетингов для ключа.
  std::string getSetting(const std::string & key ) const throw ();
  // возвращает строку из ресурса для ключа.
  std::string getString(const std::string& key) const throw ();

  // возвращает закэшированный форматтер.
  OutputFormatter* getFormatter(const std::string& key)throw();

  #ifdef SMSC_DEBUG
  void dump(std::ostream & outStream) const;
  #endif //#ifdef SMSC_DEBUG

private:
  typedef std::map <std::string, std::string> _stringmap;
  _stringmap settings;
  _stringmap resources;
  std::map<std::string,OutputFormatter*> formatters;

  void processParams(const DOM_Element &elem, _stringmap & settings, const std::string &prefix) throw ();
};

};//resourcemanager
};//smsc

#endif
