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

#include <util/debug.h>
#include <scag/re/actions/MainActionFactory.h>
#include <scag/util/singleton/Singleton.h>
#include <logger/Logger.h>


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


class RuleEngineImpl : RuleEngine
{

    MainActionFactory factory;
    std::string RulesDir;
    Logger * logger;

    friend struct RulesReference; 
    struct Rules
    {
        Mutex           rulesLock;
        IntHash<Rule*>  rules;
        int             useCounter;

        Rules() : useCounter(1) {}
        ~Rules() 
         {
             IntHash<Rule*>::Iterator it = rules.First();
             int ruleId; Rule* rule = 0;
             while (it.Next(ruleId, rule))
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

        IntHash<Rule*>& operator->() 
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
        IntHash<Rule*>::Iterator it = rules->rules.First();
        int oldRuleId; Rule* rule = 0;
        while (it.Next(oldRuleId, rule))
        {
            if (!rule) continue;
            rule->ref();
            newRules->rules.Insert(oldRuleId, rule);
        }

        return newRules;
    }

    Rule * ParseFile(const std::string& xmlFile);
    bool isValidFileName(std::string fname,int& ruleId);
    std::string CreateRuleFileName(const std::string& dir,const int ruleId) const;
public:
    RuleEngineImpl();
    ~RuleEngineImpl();

    virtual ActionFactory& getActionFactory() {return factory;}
    void ProcessInit(const std::string& dir);
    virtual void updateRule(int ruleId);
    virtual void removeRule(int ruleId);
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


Rule * RuleEngineImpl::ParseFile(const std::string& xmlFile)
{
    int errorCount = 0;
    int errorCode = 0;

    SAXParser* parser = new SAXParser;
    XMLBasicHandler handler(factory);

    try
    {
        parser->setValidationScheme(SAXParser::Val_Always);
        parser->setDoSchema(true);
        parser->setValidationSchemaFullChecking(true);
        parser->setDoNamespaces(true);

        parser->setValidateAnnotations(false);   

        parser->setValidationConstraintFatal(true);

        parser->setDocumentHandler(&handler);
        parser->setErrorHandler(&handler);


        parser->parse(xmlFile.c_str());
        errorCount = parser->getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        smsc_log_error(logger,"XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"An error occurred. Error: %s", msg.localForm());
    }
    catch (RuleEngineException& e)
    {
        if (e.getLineNumber()>0) smsc_log_error(logger,"Error at line %d: %s",e.getLineNumber(), e.what());
        else smsc_log_error(logger,"Error: %s",e.what());
    }
    catch (SCAGException& e)
    {
        smsc_log_error(logger,"Terminate parsing Rule: %s",e.what());
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing Rule: unknown fatal error");
    }

    delete parser;

    if (errorCount > 0) 
    {
        smsc_log_error(logger,"Error parsing Rule: some errors occured");
        return 0;
    }

    Rule * rule = handler.ReturnFinalObject();
    return rule;


}

bool RuleEngineImpl::isValidFileName(std::string fname, int& ruleId)
{
    if (fname.substr(0,5).compare("rule_")) return false;
    if (fname.substr(fname.size()-4,4).compare(".xml")) return false;

    std::string str = fname.substr(5,fname.size()-5-4);

    if ((str[0] == '0')&&(str.length() > 1)) return false;

    for (int i = 1; i < str.length(); i++) 
    {
        if ((str[i] < '0')||(str[i] > '9')) return false;
    }

    ruleId = atoi(str.c_str());

    return (ruleId >= 0);
}

std::string RuleEngineImpl::CreateRuleFileName(const std::string& dir,const int ruleId) const
{
    char buff[100];
    sprintf(buff,"%d",ruleId);
    std::string result = dir;

    result.append("/rule_");
    result.append(buff);
    result.append(".xml");

    return result;   
}


RuleStatus RuleEngineImpl::process(SCAGCommand& command, Session& session)
{
    RulesReference rulesRef = getRules();
    RuleStatus rs;

    int ruleId = command.getRuleId();

    smsc_log_debug(logger,"Process RuleEngine with ruleId: %d",ruleId);

    if (rulesRef.rules->rules.Exist(ruleId)) 
    {
        Rule * rule = rulesRef.rules->rules.Get(ruleId);
        rs = rule->process(command, session);
    } 
    else
        throw RuleEngineException(0,"Cannot process Rule with ID = %d : Rule not fond%s",ruleId);

        
    return rs;
}



void RuleEngineImpl::ProcessInit(const std::string& dir)
{
    logger = Logger::getInstance("scag.re");

    smsc_log_info(logger,"");
    smsc_log_info(logger,"Rule Engine initialization...");

    RegExp::InitLocale();

    rules = new Rules();

    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        throw RuleEngineException(0,"Error during initialization XMLPlatform: %s", msg.localForm());
    }

    RulesDir = dir;

    DIR * pDir = 0;
    dirent * pDirEnt = 0;
    int ruleId = 0;

    pDir = opendir(dir.c_str());
    if (!pDir) throw RuleEngineException(0,"Invalid directory %s",dir.c_str());

    while (pDir) 
    {
         pDirEnt = readdir(pDir);
         if (pDirEnt) 
         {
             if (isValidFileName(pDirEnt->d_name,ruleId)) 
             {
                 smsc_log_debug(logger,"Rule ID is %d",ruleId);
                 updateRule(ruleId);
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
    smsc_log_info(logger,"Rule Engine inited successfully...");
    smsc_log_info(logger,"");
}


void RuleEngineImpl::updateRule(int ruleId)
{
    MutexGuard mg(changeLock);

    Rule* newRule = ParseFile(CreateRuleFileName(RulesDir,ruleId));
    if (!newRule) return;


    Rules *newRules = copyReference();
    Rule** rulePtr = newRules->rules.GetPtr(ruleId);

    if (rulePtr) 
    {
        (*rulePtr)->unref();
        newRules->rules.Delete(ruleId);
    }

    newRules->rules.Insert(ruleId, newRule);
    changeRules(newRules);      
}


void RuleEngineImpl::removeRule(int ruleId)
{
    MutexGuard mg(changeLock);

    Rule** rulePtr = rules->rules.GetPtr(ruleId);  // Can we do such direct access? TODO: Ensure
    if (!rulePtr) 
    {
        throw SCAGException("Invalid rule id %d to remove",ruleId);
        //smsc_log_warn(logger,"Invalid rule id %d to remove",ruleId);
        //return;
    }
            
    Rules *newRules = copyReference();
    rulePtr = newRules->rules.GetPtr(ruleId);
    (*rulePtr)->unref();
    newRules->rules.Delete(ruleId);
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
