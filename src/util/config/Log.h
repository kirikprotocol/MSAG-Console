#ifndef LOGGERCONFIGURATION_H_INCLUDED_C3A83759
#define LOGGERCONFIGURATION_H_INCLUDED_C3A83759

#include <xercesc/dom/DOM_Node.hpp>

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
	Log(DOM_Node & config_node);
	/**
	 * Возвращает путь к файлам лога
	 *
	 * @return путь к файлам лога
	 */
	const char *getLocation() const {return location;};
	/**
	 * устанавливает новый путь к файлам лога
	 *
	 * @param new_location
	 *               новый путь к файлам лога
	 */
	void setLocation(const char * const new_location);
private:
	char* location;
	DOM_Node *node;
};

}
}
}

#endif /* LOGGERCONFIGURATION_H_INCLUDED_C3A83759 */
