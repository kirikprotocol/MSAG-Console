#include "Manager.h"
#include <xercesc/dom/DOM.hpp>
#include "logger/Logger.h"
#include "util/xml/init.h"
#include "util/xml/utilFunctions.h"
#include "util/debug.h"
#include "util/xml/DOMTreeReader.h"
#include "util/findConfigFile.h"

namespace smsc   {
namespace util   {
namespace config {

using namespace std;
using namespace xercesc;
using namespace smsc::util::xml;

std::auto_ptr<Manager> Manager::manager;

Manager::Manager(const char* argFileName)throw(ConfigException):config_filename(argFileName)
{
  readConfig();
}

void Manager::readConfig()
{
  initXerces();
  findConfigFile();
  try
  {
    __trace__("reading config...");
    DOMTreeReader reader;
    DOMDocument *document = reader.read(config_filename.c_str());
    if (document && document->getDocumentElement())
    {
      DOMElement *elem = document->getDocumentElement();
      __trace__("config readed");
      config.parse(*elem);
      __trace2__("parsed %u ints, %u booleans, %u strings",
                 config.intParams.GetCount(),
                 config.boolParams.GetCount(),
                 config.strParams.GetCount());
    } else
    {
      throw ConfigException("Parse result is null");
    }

  } catch (ParseException &e) {
    throw ConfigException("%s",e.what());
  }

}


void Manager::save()
{
  config.saveToFile( config_filename.c_str() );
}



void Manager::findConfigFile()
{
  if (config_filename.empty())
  {
    return;
  }
  config_filename=smsc::util::findConfigFile(config_filename.c_str());
}

void Manager::reinit()
{
  std::auto_ptr<Manager> oldMng(manager);
  manager.reset(new Manager(oldMng->config_filename.c_str()));
  manager->watchers.swap(oldMng->watchers);
  ParamsVector pv;
  for(WatchersVector::iterator it=manager->watchers.begin(),end=manager->watchers.end();it!=end;++it)
  {
    pv.clear();
    ConfigParamWatchType cpwt=(*it)->getWatchedParams(pv);
    bool needNotify=false;
    for(ParamsVector::iterator pit=pv.begin(),pend=pv.end();pit!=pend;++pit)
    {
      try{
        switch(pit->first)
        {
          case cvtInt:
          {
            if(oldMng->config.getInt(pit->second.c_str())!=manager->config.getInt(pit->second.c_str()))
            {
              if(cpwt==cpwtIndividual)
              {
                (*it)->paramChanged(pit->first,pit->second);
              }else
              {
                needNotify=true;
                break;
              }
            }
          }break;
          case cvtString:
          {
            if(strcmp(oldMng->config.getString(pit->second.c_str()),manager->config.getString(pit->second.c_str()))!=0)
            {
              if(cpwt==cpwtIndividual)
              {
                (*it)->paramChanged(pit->first,pit->second);
              }else
              {
                needNotify=true;
                break;
              }
            }
          }break;
          case cvtBool:
          {
            if(oldMng->config.getInt(pit->second.c_str())!=manager->config.getInt(pit->second.c_str()))
            {
              if(cpwt==cpwtIndividual)
              {
                (*it)->paramChanged(pit->first,pit->second);
              }else
              {
                needNotify=true;
                break;
              }
            }
          }break;
        }
      }catch(...)
      {
        __warning2__("watch parameter '%s' not found",pit->second.c_str());
      }
    }
    if(needNotify)
    {
      (*it)->paramsChanged();
    }
  }
}

}
}
}
