#include <stdio.h>
#include "Logger.h"
#include <log4cpp/SimpleConfigurator.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/FileAppender.hh>
#include "debug.h"

namespace smsc {
namespace util {

bool Logger::isInitialized = false;

log4cpp::Category *_trace_cat;

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
  try {
    isInitialized = false;
    log4cpp::Category::getRoot().removeAllAppenders();
    log4cpp::SimpleConfigurator::configure(configFileName);
  } catch (log4cpp::ConfigureFailure& exception) {
    fprintf( stderr, "Exception occured during configuration log4cpp: %s\n", exception.what() );
    log4cpp::Appender* appender = new log4cpp::FileAppender("FileAppender", "smsc.log");
    appender->setLayout(new log4cpp::BasicLayout());
    log4cpp::Category & cat = log4cpp::Category::getRoot();
    cat.removeAllAppenders();
    cat.addAppender(appender);
    cat.setRootPriority(log4cpp::Priority::DEBUG);
  }
  isInitialized = true;
  _trace_cat=&getCategory("trace");
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
