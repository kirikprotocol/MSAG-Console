#include "RulesEngine.h"
#include <string>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/validators/DTD/DTDValidator.hpp>
#include "XMLHandlers.h"
/////////////////////////////ActionFactory/////////////////////////

Action * ActionFactory::CreateAction(const std::string& name,const SectionParams& params) const
{
    if (name=="set") return new ActionSet(params);
    if (name=="if")  return new ActionIf(params);
    return 0;
}

void ActionFactory::FillTagHash(smsc::core::buffers::Hash<int>& TagHash) const
{
    TagHash["set"] = tgActionSection;
    TagHash["if"] = tgActionSection;
    TagHash["choose"] = tgActionSection;
}


/////////////////////////////RulesEngine////////////////////////////

RulesEngine::RulesEngine()
{
    try
    {
        XMLPlatformUtils::Initialize();
    }

    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());
        std::cout << "Error during initialization! :\n" << msg.localForm() << std::endl;
    }
}


RulesEngine::~RulesEngine()
{
    XMLPlatformUtils::Terminate();
    int key;
    Rule * value;

    for (IntHash <Rule *>::Iterator it = Rules.First(); it.Next(key, value);)
    {
        delete value;
    }

}

//Hash<Action *> * Rules::RulesActions = Rules::InitRulesActions();


int RulesEngine::ParseFile(const std::string& xmlFile)
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
    delete obj;

    return 0;
}



