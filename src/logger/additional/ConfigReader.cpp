#include "logger/additional/ConfigReader.h"

#include "util/cstrings.h"

namespace smsc {
namespace logger {

using namespace smsc::util;

ConfigReader::AppenderInfo::AppenderInfo(const char * const name, const char * const type, std::auto_ptr<Properties> params)
	:name(cStringCopy(name)), type(cStringCopy(type)), params(params.release())
{}

ConfigReader::CatInfo::CatInfo(const char * const name, const char * const initStr)
	: name(cStringCopy(name))
{
	std::auto_ptr<char> a(cStringCopy(initStr));
	char * pos = strchr(a.get(), ',');
	if (pos == NULL)
	{
		level.reset(cStringCopy(trim(a.get())));
		appender.reset(cStringCopy(""));
	}
	else
	{
		*pos=0;
		level.reset(cStringCopy(trim(a.get())));
		appender.reset(cStringCopy(trim(pos+1)));
	}
}

ConfigReader::ConfigReader(Properties const & properties)
{
	std::auto_ptr<Properties> appendersSection(properties.getSection("appender"));
	char * key;
	PropertiesValue value;
	for (Properties::Iterator i = appendersSection->getIterator(); i.Next(key, value); )
	{
		char * name = NULL;
		std::auto_ptr<char> nm(cStringCopy(key));
		char * pos = strchr(nm.get(), '.');
		if (pos != NULL)
			*pos = 0;
		name = trim(nm.get());

		if (!appenders.Exists(name))
		{
			std::auto_ptr<Properties> s = appendersSection->getSection(name);
			appenders.Insert(name, createAppender(name, *s));
		}
	}

	std::auto_ptr<Properties> catsSection(properties.getSection("cat"));
	for (Properties::Iterator i = catsSection->getIterator(); i.Next(key, value); )
	{
		cats.Insert(key, new CatInfo(key, value));
	}

	if (properties.Exists("root"))
	{
		std::auto_ptr<char> rootStr(cStringCopy(properties["root"]));
		char * pos = strchr(rootStr.get(), ',');
		if (pos == NULL)
		{
			rootLevel.reset(cStringCopy(trim(rootStr.get())));
			rootAppender.reset(cStringCopy(""));
		}
		else
		{
			*pos=0;
			rootLevel.reset(cStringCopy(trim(rootStr.get())));
			rootAppender.reset(cStringCopy(trim(pos+1)));
		}
	}
	else
	{
		rootLevel.reset(cStringCopy("NOTSET"));
		rootAppender.reset(cStringCopy(""));
	}
}

ConfigReader::~ConfigReader()
{
	{
		char * key;
		CatInfo * val;
		for (CatInfos::Iterator i = cats.getIterator(); i.Next(key, val); ) 
			delete val;
		cats.Empty();
	}
	{
		char * key;
		AppenderInfo * val;
		for (AppenderInfos::Iterator i = appenders.getIterator(); i.Next(key, val); ) 
			delete val;
		appenders.Empty();
	}
}

ConfigReader::AppenderInfo* ConfigReader::createAppender(const char * const name, const Properties & ap)
{
	// хуерага
	char * k;
	const char * v;
	Properties::Iterator i = ap.getIterator(); 
	if (i.Next(k, v))
	{
		std::auto_ptr<char> t(cStringCopy(k));
		char * p = strchr(t.get(), '.');
		if (p!= NULL)
			*p = 0;

		return new AppenderInfo(name, t.get(), ap.getSection(t.get()));
	}
	else
		return NULL;
}


}
}
