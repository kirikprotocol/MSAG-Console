#ifndef __LOCALE_RESOURCES_HPP__
#define __LOCALE_RESOURCES_HPP__

#include <string>
#include <map>
#include <xercesc/dom/DOM_Element.hpp>

namespace smsc{
namespace resourcemanager{

class LocaleResources{
public:
	LocaleResources(const std::string &filename) throw ();

  // ���������� ������ �� �������� ��� �����.
  std::string getSetting(const std::string & key ) const throw ();
  // ���������� ������ �� ������� ��� �����.
  std::string getString(const std::string& key) const throw ();

	#ifdef SMSC_DEBUG
	void dump(std::ostream & outStream) const;
	#endif //#ifdef SMSC_DEBUG

private:
	typedef std::map <std::string, std::string> _stringmap;
	_stringmap settings;
	_stringmap resources;

	void processParams(const DOM_Element &elem, _stringmap & settings, const std::string &prefix) throw ();
};

};//resourcemanager
};//smsc

#endif

