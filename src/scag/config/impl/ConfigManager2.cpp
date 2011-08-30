#include <fstream>
#include <sys/stat.h>
#include <typeinfo>

#include "ConfigManager2.h"
#include "core/buffers/IntHash.hpp"
#include "license/check/license.hpp"
#include "logger/Logger.h"
#include "logger/Logger.h"
#include "scag/config/base/ConfigListener2.h"
#include "scag/config/base/ConfigView.h"
#include "scag/config/route/RouteConfig.h"
#include "scag/config/stat/StatManConfig.h"
#include "scag/util/singleton/XercesSingleton.h"
#include "util/crc32.h"
#include "util/debug.h"
#include "util/findConfigFile.h"
#include "util/findConfigFile.h"
#include "util/xml/DOMTreeReader.h"
#include "util/xml/init.h"
#include "util/xml/utilFunctions.h"

namespace scag2   {
namespace config {

using namespace std;
using namespace xercesc;
using namespace smsc::util;
using namespace smsc::util::xml;
using smsc::core::buffers::IntHash;
using smsc::core::buffers::Array;
using smsc::util::findConfigFile;
using namespace scag::config;

std::auto_ptr<char> ConfigManagerImpl::config_filename;
Hash<std::string> ConfigManagerImpl::licconfig;
LicenseInfo ConfigManagerImpl::license;
time_t ConfigManagerImpl::licenseFileMTime=0;
std::string ConfigManagerImpl::licenseFile;
std::string ConfigManagerImpl::licenseSig;

Config ConfigManagerImpl::config;

//==============================================================
//============== ConfigManager implementation ==================

ConfigManagerImpl::ConfigManagerImpl() throw (ConfigException) :
ConfigManager(),
logger(smsc::logger::Logger::getInstance("cfgman"))
{
}

void ConfigManagerImpl::Init() throw(ConfigException)
{
    util::singleton::XercesSingleton::Instance();
    // initXerces();
  //findConfigFile();
  try
  {
    __trace__("reading config...");
    DOMTreeReader reader;

    const char* cfgFile=smsc::util::findConfigFile("config.xml");
    char * filename = new char[::strlen(cfgFile) + 1];
    ::strcpy(filename, cfgFile);
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
    if(getRouteConfig_().load(smsc::util::findConfigFile("smpp_routes.xml"))!=scag::config::RouteConfig::success)
        throw Exception("Failed to load smpp routes config");
    
    getBillManConfig_().init(ConfigView(config, "BillingManager"));
    getSessionManConfig_().init(ConfigView(config, "SessionManager"));
    getStatManConfig_().init(ConfigView(config, "StatisticsManager"));
    getHttpManConfig_().init(ConfigView(config, "HttpTransport"));    
      getPersClientConfig_().init(ConfigView(config,"PvssClient"));
    getLongCallManConfig_().init(ConfigView(config, "LongCallManager"));        
    
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
    smsc_log_debug(logger, "Registering config listener. type=%d", type);
    MutexGuard mg(listenerLock);

    if(!listeners.Exist(type))
        listeners.Insert(type, listener);
        
    smsc_log_debug(logger, "Config listener registered. type=%d", type);
}

void ConfigManagerImpl::removeListener(ConfigType type)
{
    smsc_log_debug(logger, "Removing config listener. type=%d", type);
    MutexGuard mg(listenerLock);

    if(listeners.Exist(type)){
//        ConfigListener * listener = listeners.Get(type);
//        delete listener;
        listeners.Delete(type);
    }
    smsc_log_debug(logger, "Config listener removed. type=%d", type);    
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

    if(  getStatManConfig_().check(ConfigView(config, "StatisticsManager"))  ){
        getStatManConfig_().init(ConfigView(config, "StatisticsManager"));
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

    if(  getHttpManConfig_().check(ConfigView(config, "HttpTransport"))  ){
        getHttpManConfig_().init(ConfigView(config, "HttpTransport"));
        changedConfigs.Push(HTTPMAN_CFG);
    }
    
    if(  getPersClientConfig_().check(ConfigView(config, "Personalization"))  ){
        getPersClientConfig_().init(ConfigView(config, "Personalization"));
        changedConfigs.Push(PERSCLIENT_CFG);
    }
    
    if(  getLongCallManConfig_().check(ConfigView(config, "LongCallManager"))  ){
        getPersClientConfig_().init(ConfigView(config, "LongCallManager"));
        changedConfigs.Push(LONGCALLMAN_CFG);
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

void ConfigManagerImpl::checkLicenseFile()
{
  if(licenseFile.length()==0)
  {
      licenseFile=smsc::util::findConfigFile("license.ini");
      licenseSig=smsc::util::findConfigFile("license.sig");
  }
  struct stat fst;
  if(::stat(licenseFile.c_str(),&fst)!=0)
  {
    throw Exception("License file not found:'%s'",licenseFile.c_str());
  }
  if(fst.st_mtime==licenseFileMTime)
  {
    return;
  }
  static const char *lkeys[]=
  {
  "Organization",
  "Hostids",
  "MaxSmsThroughput",
  "MaxHttpThroughput",
  "MaxMmsThroughput",
  "LicenseExpirationDate",
  "LicenseType",
  "Product"
  };
  if (!smsc::license::check::CheckLicense(licenseFile.c_str(),licenseSig.c_str(),licconfig,lkeys,sizeof(lkeys)/sizeof(lkeys[0])))
  {
    smsc_log_error(logger, "Invalid license");
    throw Exception("Invalid license");
  }
  licenseFileMTime=fst.st_mtime;

  license.maxsms=atoi(licconfig["MaxSmsThroughput"].c_str());
  license.maxhttp=atoi(licconfig["MaxHttpThroughput"].c_str());
  license.maxmms=atoi(licconfig["MaxMmsThroughput"].c_str());
  int y,m,d;
  sscanf(licconfig["LicenseExpirationDate"].c_str(),"%d-%d-%d",&y,&m,&d);
  struct tm t={0,};
  t.tm_year=y-1900;
  t.tm_mon=m-1;
  t.tm_mday=d;
  license.expdate = mktime(&t);
    // long hostid;

    if ( !smsc::license::check::checkHostIds(licconfig["Hostids"].c_str()) ) {
        throw std::runtime_error("code 1");
    }

    /*
  std::string ids=licconfig["Hostids"];
  std::string::size_type pos=0;
  bool ok = false;
    char hostid[200];
    smsc::license::check::gethostid(hostid,sizeof(hostid));

  do {
      char buf[200];
      sscanf(ids.c_str()+pos,"%150s", &buf);

      if ( 0 == strcmp(hostid,buf) ) {
          ok = true;
          break;
      }

    pos = ids.find(',', pos);
    if (pos!=std::string::npos) pos++;

  } while(pos!=std::string::npos);

  if (!ok) throw runtime_error("code 1");
     */

  if (smsc::util::crc32(0,licconfig["Product"].c_str(),licconfig["Product"].length())!=0x1D5DA434) throw runtime_error("code 2");

  if(license.expdate < time(NULL))
  {
    char x[]=
    {
    'L'^0x4c,'i'^0x4c,'c'^0x4c,'e'^0x4c,'n'^0x4c,'s'^0x4c,'e'^0x4c,' '^0x4c,'E'^0x4c,'x'^0x4c,'p'^0x4c,'i'^0x4c,'r'^0x4c,'e'^0x4c,'d'^0x4c,
    };
    std::string s;
    for(unsigned i=0;i<sizeof(x);i++)
    {
      s+=x[i]^0x4c;
    }
    throw runtime_error(s);
  }
}

}
}
