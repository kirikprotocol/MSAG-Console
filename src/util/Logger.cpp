#include "Log4cppInit.h"
#include <log4cpp/SimpleConfigurator.hh>

log4cpp::Category & log4cpp::Logger::getCategory(const std::string & name)				
{
	if (!isInitialized)
	{
		Init("log4cpp.init");
	}
	return Category::getInstance(name);
}

void log4cpp::Logger::Init(const std::string &configFileName)
{
	SimpleConfigurator::configure(configFileName);
	if (!isInitialized)
	{
		isInitialized = true;
	}
}

bool log4cpp::Logger::isInitialized = false;
