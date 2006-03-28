#include <sys/types.h>
#include <dirent.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <core/synchronization/Mutex.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/XHash.hpp>

#include <util/debug.h>
#include <scag/re/actions/MainActionFactory.h>
#include <scag/util/singleton/Singleton.h>
#include <logger/Logger.h>
#include <locale.h>


#include "Rule.h"
#include "RuleEngine.h"
#include "XMLHandlers.h"
#include "util/regexp/RegExp.hpp"



namespace scag { namespace re {

using smsc::core::synchronization::MutexGuard;
using namespace smsc::util;
using namespace scag::re::actions;
using namespace scag::util::singleton;
using namespace smsc::util::regexp;


using smsc::core::synchronization::Mutex;
using smsc::core::buffers::IntHash;
using namespace scag::re::actions;
using smsc::logger::Logger;

struct RulesReference;
struct Rules;



struct HashFunc{
  static unsigned int CalcHash(const RuleKey& key)
  {
    return key.serviceId + key.transport;
  }
};





class RuleEngineImpl : RuleEngine
{

    typedef XHash <RuleKey, Rule*, HashFunc> CRulesHash;

    MainActionFactory factory;
    std::string RulesDir;
    Logger * logger;

    friend struct RulesReference; 
    struct Rules
    {
        Mutex           rulesLock;
        CRulesHash  rules;
        int             useCounter;

        Rules() : useCounter(1) {}
        ~Rules() 
         {
             rules.First();

             CRulesHash::Iterator it = rules.getIterator();
             RuleKey key; Rule* rule = 0;
             while (it.Next(key, rule))
             {
                 if (!rule) continue;
                 rule->unref();
             }
         }

         void ref() 
         {
             MutexGuard mg(rulesLock);
             useCounter++;
         }

         void unref() 
         {
             bool del=false;
             {
                 MutexGuard mg(rulesLock);
                 del = (--useCounter == 0);
             }
             if (del) delete this;
         }
    };

    struct RulesReference
    {
        Rules*  rules;

        RulesReference(Rules* _rules) : rules(_rules) 
        {
            __require__(rules);
            rules->ref();
        };

        RulesReference(const RulesReference& rr) : rules(rr.rules) 
        {
            __require__(rules);
            rules->ref();
        }

        ~RulesReference() 
        {
            __require__(rules);
            rules->unref();
        }

        CRulesHash& operator->() 
        {
            __require__(rules);
            return rules->rules;
        }
    };

    Mutex  rulesLock;
    Rules* rules;

    RulesReference getRules() 
    {
    MutexGuard mg(rulesLock);
    return RulesReference(rules);
    }

    void changeRules(Rules* _rules) 
    {
        MutexGuard mg(rulesLock);
        __require__(_rules);
        rules->unref();
        rules = _rules;
    }

    Mutex   changeLock; 

    Rules* copyReference()
    {
        Rules* newRules = new Rules();
        rules->rules.First();

        CRulesHash::Iterator it = rules->rules.getIterator();
        RuleKey oldKey; Rule* rule = 0;
        while (it.Next(oldKey, rule))
        {
            if (!rule) continue;
            rule->ref();
            newRules->rules.Insert(oldKey, rule);
        }

        return newRules;
    }

    Rule * ParseFile(const std::string& xmlFile);
    bool isValidFileName(std::string fname,int& ruleId);
    std::string CreateRuleFileName(const std::string& dir,const RuleKey& key) const;

    void ReadRulesFromDir(TransportType transport, const char * dir);

public:
    RuleEngineImpl();
    ~RuleEngineImpl();

    virtual ActionFactory& getActionFactory() {return factory;}
    void ProcessInit(const std::string& dir);
    virtual void updateRule(RuleKey& key);
    virtual void removeRule(RuleKey& key);
    virtual RuleStatus process(SCAGCommand& command, Session& session);

};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


static bool  bRuleEngineInited = false;
static Mutex initRuleEngineLock;

inline unsigned GetLongevity(RuleEngineImpl*) { return 7; } // ? Move upper ? 
typedef SingletonHolder<RuleEngineImpl> SingleRE;

RuleEngine& RuleEngine::Instance()
{
    /*if (!bRuleEngineInited) 
    {
        MutexGuard guard(initRuleEngineLock);
        if (!bRuleEngineInited) 
            throw SCAGException("RuleEngine not inited!");
    }       */
    return SingleRE::Instance();
}



void RuleEngine::Init(const std::string& dir)
{
    if (!bRuleEngineInited)
    {
        MutexGuard guard(initRuleEngineLock);
        if (!bRuleEngineInited) {
            RuleEngineImpl& re = SingleRE::Instance();
            re.ProcessInit(dir); 
            bRuleEngineInited = true;
        }
    }
}


void RuleEngineImpl::ReadRulesFromDir(TransportType transport, const char * dir)
{
    DIR * pDir = 0;
    dirent * pDirEnt = 0;
    int serviceId = 0;

    pDir = opendir(dir);
    if (!pDir) throw RuleEngineException(0,"Invalid directory %s", dir);

    while (pDir) 
    {
        pDirEnt = readdir(pDir);
        if (pDirEnt) 
        {
            if (isValidFileName(pDirEnt->d_name, serviceId)) 
            {
                smsc_log_debug(logger,"Rule ID is %d", serviceId);
                RuleKey key;
                key.transport = transport;
                key.serviceId = serviceId;
                updateRule(key);
            }
            else if ((strcmp(pDirEnt->d_name,".")!=0)&&(strcmp(pDirEnt->d_name,"..")!=0)) 
            {
                smsc_log_error(logger,"Skipped '%s' file: Invalid file name",pDirEnt->d_name);
            }
        } 
        else 
            break;
    }

    closedir(pDir);
}


Rule * RuleEngineImpl::ParseFile(const std::string& xmlFile)
{
    int errorCount = 0;
    int errorCode = 0;

    SAXParser parser;
    XMLBasicHandler handler(factory,"KOI8-R");

    try
    {
        parser.setValidationScheme(SAXParser::Val_Always);
        parser.setDoSchema(true);
        parser.setValidationSchemaFullChecking(true);
        parser.setDoNamespaces(true);

        parser.setValidateAnnotations(false);   

        parser.setValidationConstraintFatal(true);

        parser.setDocumentHandler(&handler);
        parser.setErrorHandler(&handler);


        parser.parse(xmlFile.c_str());
        errorCount = parser.getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        smsc_log_error(logger,"Terminate parsing Rule: XMLPlatform: OutOfMemoryException");
        throw SCAGException("Terminate parsing Rule: XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"Terminate parsing Rule: An error occurred. Error: %s", msg.localForm());
        throw SCAGException("Terminate parsing Rule: An error occurred. Error: %s", msg.localForm());
    }
    catch (RuleEngineException& e)
    {
        if (e.getLineNumber()>0) smsc_log_error(logger,"Error at line %d: %s",e.getLineNumber(), e.what());
        else smsc_log_error(logger,"Error: %s",e.what());
        throw e;
    }
    catch (SCAGException& e)
    {
        smsc_log_error(logger,"Terminate parsing Rule: %s",e.what());
        throw e;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing Rule: unknown fatal error");
        throw SCAGException("Terminate parsing Rule: unknown fatal error");
    }

    //delete parser;

    if (errorCount > 0) 
    {
        smsc_log_error(logger,"Error parsing Rule: some errors occured");
        return 0;
    }

    Rule * rule = handler.ReturnFinalObject();
    return rule;


}

bool RuleEngineImpl::isValidFileName(std::string fname, int& serviceId)
{
    if (fname.substr(0,5).compare("rule_")) return false;
    if (fname.substr(fname.size()-4,4).compare(".xml")) return false;

    std::string str = fname.substr(5,fname.size()-5-4);

    if ((str[0] == '0')&&(str.length() > 1)) return false;

    for (int i = 1; i < str.length(); i++) 
    {
        if ((str[i] < '0')||(str[i] > '9')) return false;
    }

    serviceId = atoi(str.c_str());

    return (serviceId >= 0);
}

std::string RuleEngineImpl::CreateRuleFileName(const std::string& dir,const RuleKey& key) const
{
    char buff[100];

    char * transportkey = 0;
    scag::transport::TransportType value;

    scag::transport::SCAGCommand::TransportTypeHash.First();
    for (Hash <scag::transport::TransportType>::Iterator it = scag::transport::SCAGCommand::TransportTypeHash.getIterator(); it.Next(transportkey, value);)
    {
        if (value == key.transport) break;
    }

    sprintf(buff,"%d", key.serviceId);
    std::string result = dir;

    result.append("/");
    result.append(transportkey);
    result.append("/rule_");
    result.append(buff);
    result.append(".xml");

    return result;   
}


RuleStatus RuleEngineImpl::process(SCAGCommand& command, Session& session)
{
    smsc_log_debug(logger,"Process RuleEngine with serviceId: %d", command.getServiceId());

    RulesReference rulesRef = getRules();
    RuleStatus rs;

    //int ruleId = command.getRuleId();

    RuleKey key;
    key.transport = command.getType();
    key.serviceId = command.getServiceId();

    Rule ** rulePtr = rulesRef.rules->rules.GetPtr(key);

    if (rulePtr) 
    {
        rs = (*rulePtr)->process(command, session);
    } 
    else
        throw RuleEngineException(0,"Cannot process Rule with ID=%d: Rule not found", key.serviceId);

        
    return rs;
}



void RuleEngineImpl::ProcessInit(const std::string& dir)
{
    logger = Logger::getInstance("scag.re");

    smsc_log_info(logger,"");
    smsc_log_info(logger,"Rule Engine initialization...");
    
    setlocale(LC_ALL,"ru_RU.KOI8-R");
    //setlocale(LC_ALL,"UTF-8");

    //RegExp::InitLocale();

    rules = new Rules();

    try
    {
        XMLPlatformUtils::Initialize("ru_RU.KOI8-R");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        throw RuleEngineException(0,"Error during initialization XMLPlatform: %s", msg.localForm());
    }

    RulesDir = dir;


    char * transportkey = 0;
    scag::transport::TransportType value;

    std::string currentDir;

    scag::transport::SCAGCommand::TransportTypeHash.First();
    for (Hash <scag::transport::TransportType>::Iterator it = scag::transport::SCAGCommand::TransportTypeHash.getIterator(); it.Next(transportkey, value);)
    {
        currentDir = dir;
        currentDir.append("/");
        currentDir.append(transportkey);

        smsc_log_info(logger,"Rule Engine: Init transport %s...", transportkey);
        ReadRulesFromDir(value, currentDir.c_str());
        smsc_log_info(logger,"Rule Engine: Transport %s inited", transportkey);
    }

    smsc_log_info(logger,"Rule Engine inited successfully...");
    smsc_log_info(logger,"");
}


void RuleEngineImpl::updateRule(RuleKey& key)
{
    MutexGuard mg(changeLock);

    std::string filename = CreateRuleFileName(RulesDir,key);
    Rule* newRule = ParseFile(filename);
    if (!newRule) throw SCAGException("Cannod load rule %d from file %s", key.serviceId, filename.c_str());



    Rules *newRules = copyReference();
    Rule** rulePtr = newRules->rules.GetPtr(key);

    if (rulePtr) 
    {
        (*rulePtr)->unref();
        newRules->rules.Delete(key);
    }

    newRules->rules.Insert(key, newRule);
    changeRules(newRules);      
}


void RuleEngineImpl::removeRule(RuleKey& key)
{
    MutexGuard mg(changeLock);

    Rule** rulePtr = rules->rules.GetPtr(key);  // Can we do such direct access? TODO: Ensure
    if (!rulePtr) 
    {
        throw SCAGException("Invalid rule id %d to remove", key.serviceId);
        //smsc_log_warn(logger,"Invalid rule id %d to remove",ruleId);
        //return;
    }
            
    Rules *newRules = copyReference();
    rulePtr = newRules->rules.GetPtr(key);
    (*rulePtr)->unref();
    newRules->rules.Delete(key);
    changeRules(newRules);
}

RuleEngineImpl::RuleEngineImpl() : rules(0)
{
}

RuleEngineImpl::~RuleEngineImpl()
{
    XMLPlatformUtils::Terminate();
    if (rules) rules->unref();

    smsc_log_debug(logger,"Rule Engine released");
}


}}
