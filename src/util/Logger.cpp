#include "Logger.h"
#include <log4cpp/SimpleConfigurator.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/FileAppender.hh>

namespace smsc {
namespace util {

bool Logger::isInitialized = false;

/*!
 * retrieves log4cpp::Category instance for given category name
 * \param name Category name to retrieve
 * \return log4cppCategory logger category
 */
log4cpp::Category & Logger::getCategory(const std::string & name)				
{
	if (!isInitialized)
	{
		Init("log4cpp.init");
	}
	return log4cpp::Category::getInstance(name);
}

#ifdef SMSC_DEBUG
static const unsigned TRACESIZE = 10;
void BackTrace(void** trace)
{
#define TRACE_BACK(n) \
  trace[n]=__builtin_return_address(n+2);\
  if(!trace[n])return;\
  //if(trace[n]==threadstart){trace[n]=0;return;}
  TRACE_BACK(0)
  TRACE_BACK(1)
  TRACE_BACK(2)
  TRACE_BACK(3)
  TRACE_BACK(4)
  TRACE_BACK(5)
  TRACE_BACK(6)
  TRACE_BACK(7)
  TRACE_BACK(8)
  TRACE_BACK(9)
}
static void PrintTrace()
{
  void* trace[TRACESIZE];
  BackTrace(trace);
	fprintf(stderr,"***********************\n");
  for(int i=0;i<TRACESIZE;i++)
  {
    if(!trace[i])break;
    fprintf(stderr,"{0x%08X}\n",trace[i]);
  }
}
#endif

/*!
 * �������������� log4cpp::Logger �� ������� ����� ������������.
 * ������������� ���������� ������ ���� log4cpp �� ��� ������������������ �� �����.
 * ���� ���� ������������ �� ������, ��� ��������� �����-������ ������ ���
 * �������������, �� log4cpp ���������������� ����������� �� ���������
 * (���� smsc.log � ������� ����������, ������� DEBUG)
 * \param configFileName ��� ����� ������������ log4cpp
 */
void Logger::Init(const std::string &configFileName)
{
	#ifdef SMSC_DEBUG
		//PrintTrace();
	#endif
	try {
		isInitialized = false;
		log4cpp::Category::getRoot().removeAllAppenders();
		log4cpp::SimpleConfigurator::configure(configFileName);
	} catch (...) {
		log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", "smsc.log");
		appender->setLayout(new log4cpp::BasicLayout());
		log4cpp::Category & cat = log4cpp::Category::getRoot();
		cat.removeAllAppenders();
		cat.addAppender(appender);
		cat.setRootPriority(log4cpp::Priority::DEBUG);
	}
	isInitialized = true;
}

/*!
 * ���������������� log4cpp. ����� ����� ��� ����� ����� ����������������.
 */
void Logger::Shutdown()
{
	log4cpp::Category::shutdown();
	isInitialized = false;
}

}
}
