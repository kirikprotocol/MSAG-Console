#ifndef LOG4CPPINIT_H_INCLUDED_C3A87A6B
#define LOG4CPPINIT_H_INCLUDED_C3A87A6B

#include <string>
#include <log4cpp/Category.hh>

namespace smsc {
namespace util {
class Logger
{
public:
	/*!
	 * retrieves log4cpp::Category instance for given category name
	 * \param name Category name to retrieve
	 * \return log4cppCategory logger category
	 */
	static log4cpp::Category & getCategory(const std::string &name);

	/*!
	 * �������������� log4cpp::Logger �� ������� ����� ������������.
	 * ������������� ���������� ������ ���� log4cpp �� ��� ������������������ �� �����.
	 * ���� ���� ������������ �� ������, ��� ��������� �����-������ ������ ���
	 * �������������, �� log4cpp ���������������� ����������� �� ���������
	 * (���� smsc.log � ������� ����������, ������� DEBUG)
	 * \param configFileName ��� ����� ������������ log4cpp
	 */
	static void Init(const std::string & configFileName);

	/*!
	 * ���������������� log4cpp. ����� ����� ��� ����� ����� ����������������.
	 */
	static void Shutdown();
protected:
	static bool isInitialized;
};

}
}
#endif // ifndef LOG4CPPINIT_H_INCLUDED_C3A87A6B
