
#include "scag/re/RuleEngine.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <sys/types.h>
#include <dirent.h>
#include "XMLHandlers.h"

namespace scag { namespace re {

using smsc::core::synchronization::MutexGuard;
using namespace std;
using namespace smsc::util;
using namespace scag::re::actions;


void RuleEngine::updateRule(int ruleId)
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


bool RuleEngine::removeRule(int ruleId)
{
    MutexGuard mg(changeLock);

    Rule** rulePtr = rules->rules.GetPtr(ruleId);  // Can we do such direct access? TODO: Ensure
    if (!rulePtr) return false;
            
    Rules *newRules = copyReference();
    rulePtr = newRules->rules.GetPtr(ruleId);
    (*rulePtr)->unref();
    newRules->rules.Delete(ruleId);
    changeRules(newRules);
    return true;
}


void RuleEngine::Init(const std::string& dir)
{
    rules = new Rules();

    try
    {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        throw SCAGException("Error during initialization XMLPlatform: %s", msg.localForm());
    }

    RulesDir = dir;

    DIR * pDir = 0;
    dirent * pDirEnt = 0;
    int ruleId = 0;

    pDir = opendir(dir.c_str());
    if (!pDir) throw SCAGException("Invalid directory %s",dir.c_str());

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
         {
             closedir(pDir);
             return;
         }
    }

    closedir(pDir);
}




RuleEngine::RuleEngine() : rules(0)
{
}
  
RuleEngine::~RuleEngine()
{
    XMLPlatformUtils::Terminate();
    if (rules) rules->unref();
    if (rules) delete rules;
}
 
Rule * RuleEngine::ParseFile(const std::string& xmlFile)
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

bool RuleEngine::isValidFileName(std::string fname, int& ruleId)
{
    if (fname.substr(0,5).compare("rule_")) return false;
    if (fname.substr(fname.size()-4,4).compare(".xml")) return false;

    ruleId = atoi(fname.substr(5,fname.size()-5-4).c_str());

    return (ruleId > 0);
}

std::string RuleEngine::CreateRuleFileName(const std::string& dir,const int ruleId) const
{
    char buff[100];
    sprintf(buff,"%d",ruleId);
    std::string result = dir;

    result.append("/rule_");
    result.append(buff);
    result.append(".xml");

    return result;   
}

int RuleEngine::GetRuleId(SCAGCommand& command)
{
    return 1;
}


RuleStatus RuleEngine::process(SCAGCommand& command, Session& session)
{
    RulesReference rulesRef = getRules();
    RuleStatus rs;

    int ruleId = 0;
    ruleId = GetRuleId(command);

    smsc_log_debug(logger,"Process RuleEngine with ruleId: %d",ruleId);

    if (rulesRef.rules->rules.Exist(ruleId)) 
    {
        Rule * rule = rulesRef.rules->rules.Get(ruleId);
        rs = rule->process(command, session);
    } 
    else
        throw SCAGException("Cannot process Rule with ID = %d : Rule not fond%s",ruleId);

        
    return rs;
}


}}
