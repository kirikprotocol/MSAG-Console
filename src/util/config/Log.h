#ifndef LOGGERCONFIGURATION_H_INCLUDED_C3A83759
#define LOGGERCONFIGURATION_H_INCLUDED_C3A83759

#include <xercesc/dom/DOM_Element.hpp>
#include <xercesc/dom/DOM_DOMException.hpp>

namespace smsc   {
namespace util   {
namespace config {

/**
 * ��������� �������
 *
 * @author igork
 * @see Manager
 */
class Log
{
public:
	Log(DOM_Element & config_node);
	/**
	 * ���������� ���� � ������ ����
	 *
	 * @return ���� � ������ ����
	 */
	const char * const getLocation() const {return location;};
	/**
	 * ������������� ����� ���� � ������ ����
	 *
	 * @param new_location
	 *               ����� ���� � ������ ����
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
