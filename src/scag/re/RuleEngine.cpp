
#include "scag/re/RuleEngine.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <sys/types.h>
#include <dirent.h>

namespace scag { namespace re {

using smsc::core::synchronization::MutexGuard;
using namespace std;
using namespace smsc::util;
using namespace scag::re::actions;



/*
#ifndef MEMPARSE_ENCODING
   #if defined(OS390)
      #define MEMPARSE_ENCODING "ibm-1047-s390"
   #elif defined(OS400)
      #define MEMPARSE_ENCODING "ibm037"
   #else
      #define MEMPARSE_ENCODING "ascii"
   #endif
#endif  ifndef MEMPARSE_ENCODING */
/*
XMLRuleHandler::XMLRuleHandler(const std::string SectionName)
{

}
  
*/


RuleEngine::RuleEngine(const std::string& dir)
{
    /* TODO: Implement
             1) Configure global params (if any)
             2) Scan all sub-sections & create rules for it (via RulesFactory)
    */
    RulesDir = dir;
    rules = new Rules();

    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        smsc_log_error(logger,std::string("Error during initialization XMLPlatform:") + msg.localForm());
        return;
    }

    DIR * pDir = 0;
    dirent * pDirEnt = 0;
    int ruleId = 0;

    pDir = opendir(dir.c_str());
    if (!pDir) return;

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
  
RuleEngine::~RuleEngine()
{
    XMLPlatformUtils::Terminate();
    if (rules) rules->unref();
    delete rules;
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

        smsc_log_error(logger,std::string("An error occurred. Error: ") + msg.localForm());
    }
    catch (SCAGException& e)
    {
        smsc_log_error(logger,std::string("Terminate parsing Rule: ")+e.what());
    }
    catch (...)
    {
        smsc_log_error(logger,std::string("Terminate parsing Rule: unknown fatal error"));
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
        throw SCAGException("Cannot process Rule with ID = %d%s",ruleId," : Rule not found");

        
    return rs;
}


}}
