#ifndef LOGGERCONFIGURATION_H_INCLUDED_C3A83759
#define LOGGERCONFIGURATION_H_INCLUDED_C3A83759

#include <xercesc/dom/DOM_Node.hpp>

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
	Log(DOM_Node & config_node);
	/**
	 * ���������� ���� � ������ ����
	 *
	 * @return ���� � ������ ����
	 */
	const char *getLocation() const {return location;};
	/**
	 * ������������� ����� ���� � ������ ����
	 *
	 * @param new_location
	 *               ����� ���� � ������ ����
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
