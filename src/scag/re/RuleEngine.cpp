
#include "scag/re/RuleEngine.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>


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

    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        std::cout << "Error during initialization! :\n" << msg.localForm() << std::endl;
        return;
    }


}
  
RuleEngine::~RuleEngine()
{
/*    XMLPlatformUtils::Terminate();
    int key;
    Rule * value;

    for (IntHash <Rule *>::Iterator it = Rules.First(); it.Next(key, value);)
    {
        delete value;
    }
    
    delete rules;
    */
    XMLPlatformUtils::Terminate();
    if (rules) rules->unref();
    
}
 
int RuleEngine::ParseFile(const std::string& xmlFile)
{
    int errorCount;
    int errorCode;

    SAXParser* parser = new SAXParser;
    XMLBasicHandler handler(factory);

    try
    {
        parser->setValidationScheme(SAXParser::Val_Always);
        parser->setDoSchema(true);

        parser->setValidationSchemaFullChecking(true);
        parser->setDoNamespaces(false);
        parser->setValidateAnnotations(false);   

        parser->setValidationConstraintFatal(true);

        parser->setDocumentHandler(&handler);
        parser->setErrorHandler(&handler);


        parser->parse(xmlFile.data());
        errorCount = parser->getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
        errorCode = 5;          
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        std::cout << "\nAn error occurred\n  Error: " << msg.localForm() << "\n" << std::endl;
        errorCode = 4;
    }
    catch (Exception& e)
    {
        cout << e.what() << endl;
    }


    delete parser;

    if (errorCount > 0) 
    {
        cout << "There are some errors in XML file" << endl;
        return 4;
    }

    IParserHandler * obj = handler.ReturnFinalObject();
    if (obj) delete obj;

    return 0;
}



}}
