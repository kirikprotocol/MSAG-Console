#include <xercesc/dom/DOM.hpp>
#include <fstream>
#include <sys/stat.h>

#include "logger/Logger.h"
#include "util/xml/init.h"
#include "util/xml/utilFunctions.h"
#include "util/debug.h"
#include "util/xml/DOMTreeReader.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/config/alias/aliasconf.h"
#include "scag/config/statman/StatManConfig.h"
#include "util/findConfigFile.h"
#include "scag/config/ConfigManager.h"
#include "scag/config/ConfigListener.h"

#include "core/buffers/IntHash.hpp"
#include "core/buffers/Array.hpp"
#include <scag/util/singleton/Singleton.h>

#include "scag/config/ConfigView.h"

namespace scag   {
namespace config {

using namespace std;
using namespace xercesc;
using namespace smsc::util::xml;
using smsc::core::buffers::IntHash;
using smsc::core::buffers::Array;
using namespace scag::util::singleton;
using smsc::util::findConfigFile;

class ConfigManagerImpl : public ConfigManager
{
public:
    ConfigManagerImpl() throw(ConfigException);

    virtual void registerListener(ConfigType type, ConfigListener* listener);
    virtual void removeListener(ConfigType type);

    virtual void reloadConfig(ConfigType type);
	virtual void reloadAllConfigs();

    virtual RouteConfig  getRouteConfig(){return routeCfg;};
    virtual SmppManConfig getSmppManConfig(){return smppManCfg;};
    virtual StatManConfig getStatManConfig(){return statManCfg;};
    virtual BillingManagerConfig getBillManConfig(){return billManCfg;};
    virtual SessionManagerConfig getSessionManConfig(){return sessionManCfg;};
    virtual Hash<std::string>*& getLicConfig(){return licconfig;};
    virtual Config* getConfig(){return &config;};

    void save();
protected:
    IntHash<ConfigListener*> listeners;
    Mutex listenerLock;

    SmppManConfig smppManCfg;
    RouteConfig routeCfg;
    AliasConfig aliasCfg;
    StatManConfig statManCfg;
    BillingManagerConfig billManCfg;
    SessionManagerConfig sessionManCfg;
    Hash<std::string> *licconfig;

private:
    void findConfigFile();
protected:
    Config config;

private:
    static std::auto_ptr<char> config_filename;
    void writeHeader(std::ostream &out);
    void writeFooter(std::ostream &out);
    void reload(Array<int>& changedConfigs) throw(ConfigException);

    DOMDocument * parse(const char * const filename) throw (ConfigException);

};

std::auto_ptr<char> ConfigManagerImpl::config_filename;

void ConfigManagerImpl::registerListener(ConfigType type, ConfigListener *listener)
{
    MutexGuard mg(listenerLock);

    if(!listeners.Exist(type))
        listeners.Insert(type, listener);
}

void ConfigManagerImpl::removeListener(ConfigType type)
{
    MutexGuard mg(listenerLock);

    if(listeners.Exist(type)){
        ConfigListener * listener = listeners.Get(type);
        delete listener;
        listeners.Delete(type);
    }
}

void ConfigManagerImpl::reloadConfig(ConfigType type)
{
    MutexGuard mg(listenerLock);

    switch(type){
    case ROUTE_CFG:
        routeCfg.reload();
        break;
    case SMPPMAN_CFG:
        smppManCfg.reload();
        break;
    default:
        {
            Array<int> changedConfigs;
            reload(changedConfigs);
        }
        break;
    };

    if(listeners.Exist(type)){
        ConfigListener * listener = listeners.Get(type);
        if(listener){
            listener->configChanged();
        }
    }
}

void ConfigManagerImpl::reloadAllConfigs()
{
    MutexGuard mg(listenerLock);

    routeCfg.reload();
    smppManCfg.reload();

    if(listeners.Exist(ALIAS_CFG)){
        ConfigListener * listener = listeners.Get(ALIAS_CFG);
        if(listener){
            listener->configChanged();
        }
    }

    if(listeners.Exist(ROUTE_CFG)){
        ConfigListener * listener = listeners.Get(ROUTE_CFG);
        if(listener){
            listener->configChanged();
        }
    }

    if(listeners.Exist(SMPPMAN_CFG)){
        ConfigListener * listener = listeners.Get(SMPPMAN_CFG);
        if(listener){
            listener->configChanged();
        }
    }

    Array<int> changedConfigs;
    reload(changedConfigs);

    for(int i=0; i<= changedConfigs.Count() - 1; i++){
        if(listeners.Exist(changedConfigs[i])){
            ConfigListener * listener = listeners.Get(changedConfigs[i]);
            if(listener){
                listener->configChanged();
            }
        }
    }
}

static bool  bConfigManagerInited = false;
static Mutex initConfigManagerLock;

inline unsigned GetLongevity(ConfigManagerImpl*) { return 8; } 
typedef SingletonHolder<ConfigManagerImpl> SingleConfig;

void ConfigManager::Init()
{
    if (!bConfigManagerInited)
    {
        MutexGuard guard(initConfigManagerLock);

        if (!bConfigManagerInited) {
            ConfigManagerImpl& cfgman = SingleConfig::Instance();
            bConfigManagerInited = true;
        }
    }
}

ConfigManager& ConfigManager::Instance()
{
    if (!bConfigManagerInited) 
    {
        MutexGuard guard(initConfigManagerLock);
        
        if (!bConfigManagerInited) 
            throw std::runtime_error("ConfigManager not inited!");
        
    }
    return SingleConfig::Instance();
}

ConfigManagerImpl::ConfigManagerImpl()
  throw(ConfigException)
{

  initXerces();
  findConfigFile();

  try 
  {
    __trace__("reading config...");
    DOMTreeReader reader;

    char * filename = new char[strlen(smsc::util::findConfigFile("../conf/config.xml")) + 1];
    std::strcpy(filename, smsc::util::findConfigFile("../conf/config.xml"));
    config_filename = std::auto_ptr<char>(filename);

    DOMDocument *document = reader.read(config_filename.get());
    if (document && document->getDocumentElement())
    {
      DOMElement *elem = document->getDocumentElement();
      __trace__("config readed");
      config.parse(*elem);
      __trace2__("parsed %u ints, %u booleans, %u strings\n",
                 config.intParams.GetCount(),
                 config.boolParams.GetCount(),
                 config.strParams.GetCount());
    } else {
      throw ConfigException("Parse result is null");
    }

    // Inits subsystems configs

    smppManCfg.load(smsc::util::findConfigFile("../conf/sme.xml"));
    routeCfg.load(smsc::util::findConfigFile("../conf/routes.xml"));
    billManCfg.init(ConfigView(config, "BillingManager"));
    sessionManCfg.init(ConfigView(config, "SessionManager"));
    statManCfg.init(ConfigView(config, "MessageStorage"));

} catch (ParseException &e) {
    throw ConfigException(e.what()); 
}catch(ConfigException& e){
    throw ConfigException(e.what()); 
}catch(Exception &e) {
    throw ConfigException(e.what()); 
}catch(...) {
    throw ConfigException("ConfigManagerImpl exception, unknown exception"); 
} 
} 

/**
 * Create our parser, then attach an error handler to the parser.
 * The parser will call back to methods of the ErrorHandler if it
 * discovers errors during the course of parsing the XML document.
 *
 * @return created parser
 *//*
DOMParser * Manager::createParser() {
  //logger.debug("Entering createParser()");
  DOMParser *parser = new DOMParser;
  parser->setValidationScheme(DOMParser::Val_Always);
  parser->setDoNamespaces(false);
  parser->setDoSchema(false);
  parser->setValidationSchemaFullChecking(false);
  //logger.debug("  Creating ErrorReporter");
  DOMErrorLogger *errReporter = new DOMErrorLogger();
  parser->setErrorHandler(errReporter);
  parser->setCreateEntityReferenceNodes(false);
  parser->setToCreateXMLDeclTypeNode(false);
  parser->setEntityResolver(new DtdResolver());
  //logger.debug("Leaving createParser()");

  return parser;
}
*/

/*!
 * Parse the XML file, catching any XML exceptions that might propogate
 * out of it.
 */
/*
DOM_Document Manager::parse(DOMParser *parser, const char * const filename)
  throw (ConfigException)
{
  try
  {
    parser->parse(filename);
    int errorCount = parser->getErrorCount();
    if (errorCount > 0) {
      char exceptionMsg[1024];
      snprintf(exceptionMsg, sizeof(exceptionMsg), "An %d errors occured during parsing \"%s\"", errorCount, filename);
      throw ConfigException(exceptionMsg);
    }
  }
  catch (const XMLException& e)
  {
    std::auto_ptr<char> message = XMLString::transcode(e.getMessage());
    XMLExcepts::Codes code = e.getCode();
    const char *srcFile = e.getSrcFile();
    unsigned int line = e.getSrcLine();
    char exceptionMsg[1024];
    snprintf(exceptionMsg, sizeof(exceptionMsg), "An error occured during parsing \"%s\" at file \"%s\" on line %d. Nested: %d: %s", filename, srcFile, line, code, message.get());
    throw ConfigException(exceptionMsg);
  }
  catch (const DOM_DOMException& e)
  {
    char msg[1024];
    snprintf(msg, sizeof(msg), "A DOM error occured during parsing\"%s\". DOMException code: %i", filename, e.code);
    throw ConfigException(msg);
  }
  catch (...)
  {
    char msg[1024];
    snprintf(msg, sizeof(msg), "An error occured during parsing \"%s\"", filename);
    throw ConfigException(msg);
  }

  return parser->getDocument();
}


std::ostream & operator << (std::ostream & out, const XMLCh * const string)
{
  std::auto_ptr<char> p(XMLString::transcode(string));
  out << p.get();
  return out;
}
*/
/**
 * Записывает конфигурацию системы.
 */
void ConfigManagerImpl::save()
{
  std::ofstream *out = new std::ofstream(config_filename.get());
  writeHeader(*out);
  config.save(*out);
  writeFooter(*out);
  out->flush();
  if (out->fail())
  {
    cerr << "Couldn't save config to \"" << config_filename.get() << "\"\n";
  }
  out->close();
  delete out;
}

void ConfigManagerImpl::writeHeader(std::ostream &out)
{
  out << "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>" << std::endl;
  out << "<!DOCTYPE config SYSTEM \"configuration.dtd\">" << std::endl;
  out << "<config>" << std::endl;
}

void ConfigManagerImpl::writeFooter(std::ostream &out)
{
  out << "</config>" << std::endl;
}

void ConfigManagerImpl::findConfigFile()
{
  if (config_filename.get() == 0)
    return;

  struct stat s;
  std::auto_ptr<char> tmp_name(new char[strlen(config_filename.get())+10]);

  strcpy(tmp_name.get(), config_filename.get());
  if (stat(tmp_name.get(), &s) == 0)
    return;

  strcpy(tmp_name.get(), "conf/");
  strcat(tmp_name.get(), config_filename.get());
  if (stat(tmp_name.get(), &s) == 0)
  {
    config_filename = tmp_name;
    return;
  }

  strcpy(tmp_name.get(), "../conf/");
  strcat(tmp_name.get(), config_filename.get());
  if (stat(tmp_name.get(), &s) == 0)
  {
    config_filename = tmp_name;
    return;
  }
}

void ConfigManagerImpl::reload(Array<int>& changedConfigs)
        throw(ConfigException)
{

    try 
  {

    config.clean();

    __trace__("reading config...");
    DOMTreeReader reader;
    DOMDocument *document = reader.read(config_filename.get());
    if (document && document->getDocumentElement())
    {
      DOMElement *elem = document->getDocumentElement();
      __trace__("config readed");
      config.parse(*elem);
      __trace2__("parsed %u ints, %u booleans, %u strings\n",
                 config.intParams.GetCount(),
                 config.boolParams.GetCount(),
                 config.strParams.GetCount());
    } else {
      throw ConfigException("Parse result is null");
    }

    if(  statManCfg.check(ConfigView(config, "MessageStorage"))  ){
        statManCfg.init(ConfigView(config, "MessageStorage"));
        changedConfigs.Push(STATMAN_CFG);
    }

    if(  billManCfg.check(ConfigView(config, "BillingManager"))  ){
        billManCfg.init(ConfigView(config, "BillingManager"));
        changedConfigs.Push(BILLMAN_CFG);
    }

    if(  sessionManCfg.check(ConfigView(config, "SessionManager"))  ){
        sessionManCfg.init(ConfigView(config, "SessionManager"));
        changedConfigs.Push(SESSIONMAN_CFG);
    }


    } catch (ParseException &e) {
    throw ConfigException(e.what());
  }
}

}
}
