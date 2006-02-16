#include <xercesc/dom/DOM.hpp>
#include <fstream>
#include <sys/stat.h>

#include "logger/Logger.h"
#include "util/xml/init.h"
#include "util/xml/utilFunctions.h"
#include "util/debug.h"
#include "util/xml/DOMTreeReader.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/config/stat/StatManConfig.h"
#include "util/findConfigFile.h"
#include "scag/config/ConfigManager.h"
#include "scag/config/ConfigListener.h"

#include "core/buffers/IntHash.hpp"
#include "core/buffers/Array.hpp"
#include <scag/util/singleton/Singleton.h>

#include "scag/config/ConfigView.h"
#include "ConfigListener.h"
#include <typeinfo.h>
#include "logger/Logger.h"

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
    virtual ~ConfigManagerImpl() {cout << "ConfigManager released" << endl;}
    ConfigManagerImpl() throw(ConfigException);

    virtual void registerListener(ConfigType type, ConfigListener* listener);
    virtual void removeListener(ConfigType type);

    virtual void reloadConfig(ConfigType type);
  virtual void reloadAllConfigs();
  static void Init() throw(ConfigException);
  virtual RouteConfig&  getRouteConfig(){ return getRouteConfig_(); }
  virtual SmppManConfig& getSmppManConfig(){ return getSmppManConfig(); }
  virtual StatManConfig& getStatManConfig(){ return getStatManConfig_(); }
  virtual BillingManagerConfig& getBillManConfig(){ return getBillManConfig_(); }
  virtual SessionManagerConfig& getSessionManConfig(){ return getSessionManConfig_(); }

    static RouteConfig&  getRouteConfig_()
    {
        static RouteConfig routeCfg;
        return routeCfg;
    }
    static SmppManConfig& getSmppManConfig_()
    {
        static SmppManConfig smppManCfg;
        return smppManCfg;
    }
    static StatManConfig& getStatManConfig_()
    {
        static StatManConfig statManCfg;
        return statManCfg;
    }
    static BillingManagerConfig& getBillManConfig_()
    {
        static BillingManagerConfig billManCfg;
        return billManCfg;
    }
    static SessionManagerConfig& getSessionManConfig_()
    {
        static SessionManagerConfig sessionManCfg;
        return sessionManCfg;
    };
    virtual Hash<std::string>*& getLicConfig(){return licconfig;};
    virtual Config* getConfig(){return &config;};

    void save();
protected:
    IntHash<ConfigListener*> listeners;
    Mutex listenerLock;
    smsc::logger::Logger* logger;

    static Hash<std::string> *licconfig;

private:
    static void findConfigFile();
protected:
    static Config config;

private:
    static std::auto_ptr<char> config_filename;
    void writeHeader(std::ostream &out);
    void writeFooter(std::ostream &out);
    void reload(Array<int>& changedConfigs) throw(ConfigException);

    DOMDocument * parse(const char * const filename) throw (ConfigException);

};

std::auto_ptr<char> ConfigManagerImpl::config_filename;
Hash<std::string> * ConfigManagerImpl::licconfig = 0;
Config ConfigManagerImpl::config;

//==============================================================
//============== Singleton related part ========================

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
            cfgman.Init();
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

//==============================================================
//============== ConfigManager implementation ==================

ConfigManagerImpl::ConfigManagerImpl()
  throw(ConfigException)
    : logger(smsc::logger::Logger::getInstance("cfgman"))
{
}

void ConfigManagerImpl::Init()
  throw(ConfigException)
{
  initXerces();
  //findConfigFile();
  try
  {
    __trace__("reading config...");
    DOMTreeReader reader;

    const char* cfgFile=smsc::util::findConfigFile("config.xml");
    char * filename = new char[strlen(cfgFile) + 1];
    std::strcpy(filename, cfgFile);
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

    //smppManCfg.load(smsc::util::findConfigFile("sme.xml"));
    getRouteConfig_().load(smsc::util::findConfigFile("routes.xml"));
    getBillManConfig_().init(ConfigView(config, "BillingManager"));
    getSessionManConfig_().init(ConfigView(config, "SessionManager"));
    getStatManConfig_().init(ConfigView(config, "StatisticsManager"));

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

//================================================================

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

    smsc_log_info(logger, "Config %d is reloading...", type);
    smsc_log_info(logger, "Types: ROUTE - %d, SMPPMAN - %d", ROUTE_CFG, SMPPMAN_CFG);

    switch(type){
    case ROUTE_CFG:
        smsc_log_info(logger, "Route config is reloading...");
        getRouteConfig_().reload();
        smsc_log_info(logger, "Route config has reloaded");
        break;
    case SMPPMAN_CFG:
        smsc_log_info(logger, "SmppMan config is reloading...");
        smsc_log_info(logger, "SmppMan config has reloaded");
        //getSmppManConfig_().reload();
        break;
    default:
        {
            Array<int> changedConfigs;
            reload(changedConfigs);
        }
        break;
    };

    if(listeners.Exist(type)){
        smsc_log_info(logger, "Trigger on %d config is processing...", type);
        ConfigListener * listener = listeners.Get(type);
        if(listener){
            listener->configChanged();
            smsc_log_info(logger, "Trigger on %d config has processed ok", type);
        }
    }
}

void ConfigManagerImpl::reloadAllConfigs()
{
    MutexGuard mg(listenerLock);

    getRouteConfig_().reload();
    //getSmppManConfig_().reload();

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
 * ���������� ������������ �������.
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

    smsc_log_info(logger, "reading config...");
    DOMTreeReader reader;
    DOMDocument *document = reader.read(config_filename.get());
    if (document && document->getDocumentElement())
    {
      DOMElement *elem = document->getDocumentElement();
      smsc_log_info(logger, "config readed");
      config.parse(*elem);
      smsc_log_info(logger, "parsed %u ints, %u booleans, %u strings",
                 config.intParams.GetCount(),
                 config.boolParams.GetCount(),
                 config.strParams.GetCount());
    } else {
      throw ConfigException("Parse result is null");
    }

    if(  getStatManConfig_().check(ConfigView(config, "MessageStorage"))  ){
        getStatManConfig_().init(ConfigView(config, "MessageStorage"));
        changedConfigs.Push(STATMAN_CFG);
    }

    if(  getBillManConfig_().check(ConfigView(config, "BillingManager"))  ){
        getBillManConfig_().init(ConfigView(config, "BillingManager"));
        changedConfigs.Push(BILLMAN_CFG);
    }

    if(  getSessionManConfig_().check(ConfigView(config, "SessionManager"))  ){
        getSessionManConfig_().init(ConfigView(config, "SessionManager"));
        changedConfigs.Push(SESSIONMAN_CFG);
    }


    } catch (ParseException &e) {
    throw ConfigException(e.what());
  }
}

ConfigListener::ConfigListener(ConfigType type_) : type(type_)
{
    ConfigManager& cfg = ConfigManager::Instance();
    try {
        ConfigManagerImpl& cfg_ = dynamic_cast<ConfigManagerImpl&>(cfg);
        cfg_.registerListener(type, this);
    }catch(std::bad_cast& e)
    {
        throw Exception("ConfigListener exception, ConfigManagerImpl undefined.");
    }catch(...)
    {
        throw Exception("ConfigListener exception, Unknown error.");
    }
}

ConfigListener::~ConfigListener()
{
    ConfigManager& cfg = ConfigManager::Instance();
    try {
        ConfigManagerImpl& cfg_ = dynamic_cast<ConfigManagerImpl&>(cfg);
        cfg_.removeListener(type);
    }catch(std::bad_cast& e)
    {
        throw Exception("ConfigListener exception, ConfigManagerImpl undefined.");
    }catch(...)
    {
        throw Exception("ConfigListener exception, Unknown error.");
    }
}

}
}
