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
  smsc::util::auto_arr_ptr<char> a(cStringCopy(initStr));
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

void ConfigReader::init(Properties const & properties)
{
    clear();

  std::auto_ptr<Properties> appendersSection(properties.getSection("appender"));
  char * key;
  PropertiesValue value;
  for (Properties::Iterator i (appendersSection.get()); i.Next(key, value); )
  {
    char * name = NULL;
    smsc::util::auto_arr_ptr<char> nm(cStringCopy(key));
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
  for (Properties::Iterator i (catsSection.get()); i.Next(key, value); )
  {
    cats.Insert(key, new CatInfo(key, value));
  }

  if (properties.Exists("root"))
  {
    smsc::util::auto_arr_ptr<char> rootStr(cStringCopy(properties["root"]));
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
  if (properties.Exists("configReloadInterval"))
        configReloadInterval = atoi(properties["configReloadInterval"]);
}

void ConfigReader::serialize(std::string& str)
{
    str = "root=";
    str += rootLevel.get();
    if(*rootAppender.get())
    {
        str += ", ";
        str += rootAppender.get();
    }
    str += '\n';

    if(configReloadInterval)
    {
        char buf[20];
        buf[19] = 0;
        str += "configReloadInterval=";
        sprintf(buf,"%u",configReloadInterval);
        str += buf;
        str += '\n';
    }

  {
    char * key, *prop;
        PropertiesValue value;
    AppenderInfo * val;
    for (AppenderInfos::Iterator i (&appenders); i.Next(key, val);)
        {
          for (Properties::Iterator i1 ( val->params.get()); i1.Next(prop, value); )
            {
                str += "appender.";
                str += val->name.get();
                str += '.';
                str += val->type.get();
                str += '.';
                str += prop;
                str += '=';
                str += value;
                str += '\n';
            }
        }
  }
  {
    char * key;
    CatInfo * val;
    for (CatInfos::Iterator i (& cats); i.Next(key, val); )
        {
            if(!strcmp(val->level.get(), "NOTSET") && !*val->appender.get()) continue;
            str += "cat.";
            str += val->name.get();
            str += '=';
            str += val->level.get();
            if(*val->appender.get())
            {
                str += ", ";
                str += val->appender.get();

            }
            str += '\n';
        }
  }
}

ConfigReader::ConfigReader(Properties const & properties): configReloadInterval(0)
{
    init(properties);
}

void ConfigReader::clear()
{
  {
    char * key;
    CatInfo * val;
    for (CatInfos::Iterator i (&cats); i.Next(key, val); )
      delete val;
    cats.Empty();
  }
  {
    char * key;
    AppenderInfo * val;
    for (AppenderInfos::Iterator i (&appenders); i.Next(key, val); )
      delete val;
    appenders.Empty();
  }
}

ConfigReader::~ConfigReader()
{
    clear();
}

ConfigReader::AppenderInfo* ConfigReader::createAppender(const char * const name, const Properties & ap)
{
  // �������
  char * k;
  const char * v;
  Properties::Iterator i (&ap);
  if (i.Next(k, v))
  {
    smsc::util::auto_arr_ptr<char> t(cStringCopy(k));
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
