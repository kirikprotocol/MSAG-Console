#ifndef LOGGERCONFIGURATION_H_INCLUDED_C3A83759
#define LOGGERCONFIGURATION_H_INCLUDED_C3A83759

#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>

namespace smsc   {
namespace util   {
namespace config {

/**
 * Настройки логгера
 *
 * @author igork
 * @see Manager
 */
class Log
{
public:
	Log(DOM_Element & config_node);
	/**
	 * Возвращает путь к файлам лога
	 *
	 * @return путь к файлам лога
	 */
	const char * const getLocation() const {return location;};
	/**
	 * устанавливает новый путь к файлам лога
	 *
	 * @param new_location
	 *               новый путь к файлам лога
	 */
	void setLocation(const char * const new_location) throw (DOM_DOMException);
private:
	char* location;
	DOM_Element node;

	static const DOMString location_name;
};

}
}
}

#endif /* LOGGERCONFIGURATION_H_INCLUDED_C3A83759 */
