#include <stdio.h>
#include "Logger.h"
#include <log4cpp/PropertyConfigurator.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/FileAppender.hh>

namespace smsc {
namespace util {

bool Logger::isInitialized = false;

log4cpp::Category *_trace_cat=NULL;
log4cpp::Category *_map_cat=NULL;
log4cpp::Category *_mapdlg_cat=NULL;
log4cpp::Category *_mapproxy_cat=NULL;

/*!
 * retrieves log4cpp::Category instance for given category name
 * \param name Category name to retrieve
 * \return log4cppCategory logger category
 */
log4cpp::Category & Logger::getCategory(const std::string & name)
{
  if (!isInitialized)
  {
    Init("log4cpp.properties");
  }
  return log4cpp::Category::getInstance(name);
}

/*!
 * Инициализирует log4cpp::Logger по данному файлу конфигурации.
 * Инициализация происходит только если log4cpp не был проинициализирован до этого.
 * Если файл конфигурации не найден, или произошла какая-нибудь ошибка при
 * инициализации, то log4cpp инициализируется параметрами по умолчанию
 * (файл smsc.log в текущей директории, уровень DEBUG)
 * \param configFileName имя файла конфигурации log4cpp
 */
void Logger::Init(const std::string &configFileName)
{
  if(isInitialized)return;
  //fprintf(stderr,"Logger::Init: isInitialized=%s\n",isInitialized?"true":"false");
  try {
    isInitialized = false;
    log4cpp::Category::getRoot().removeAllAppenders();
    log4cpp::PropertyConfigurator::configure(configFileName);
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
  _map_cat=&getCategory("map");
  _mapdlg_cat=&getCategory("map.dialog");
  _mapproxy_cat=&getCategory("map.proxy");
}

/*!
 * Деинициализирует log4cpp. После этого его можно снова инициализировать.
 */
void Logger::Shutdown()
{
  log4cpp::Category::shutdown();
  isInitialized = false;
}

}
}
