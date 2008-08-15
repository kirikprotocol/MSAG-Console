
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/Locator.hpp>
#include <xercesc/util/TransService.hpp>

#include "XMLHandlers2.h"
#include "scag/util/encodings/Encodings.h"

namespace scag2 {
namespace re { 

//static const char*  encodingName    = "LATIN1";
// static XMLFormatter::UnRepFlags unRepFlags  = XMLFormatter::UnRep_CharRef;
// static XMLTranscoder::UnRepOpts unRepOpts = XMLTranscoder::UnRep_Throw;

static const XMLCh  gEndElement[] = { chOpenAngle, chForwardSlash, chNull };

using namespace smsc::util;
using namespace scag::util::encodings;

/////////////////////////////////////////////SemanticAnalyser/////////////////////////////


SemanticAnalyser::SemanticAnalyser(const ActionFactory& obj) :
CurrentObject(0), factory(&obj), RootObject(0)
{
}

SemanticAnalyser::~SemanticAnalyser()
{
    if (RootObject) delete RootObject;
}

Rule * SemanticAnalyser::ReturnRuleObject()
{
    Rule * result = RootObject;
    RootObject = 0;
    return result;
}

void SemanticAnalyser::DeliverBeginTag(const std::string& name,const SectionParams& params,int nLine)
{
    IParserHandler * NewObj = 0;

    if (CurrentObject) 
    {
        try
        {
            NewObj = CurrentObject->StartXMLSubSection(name,params,*factory);
        } catch (SCAGException& e)
        {
            if (NewObj) delete NewObj;
            if (RootObject)
            {
                delete RootObject;
                RootObject = 0;
            }
            throw RuleEngineException(nLine, "Invalid object '%s' to create: %s",name.c_str(),e.what());
        }

        if (NewObj) 
        {
            HistoryObjectStack.push(CurrentObject);
            CurrentObject = NewObj;
        }
    }
    else
    {
        if (RootObject) 
        {
            delete RootObject;
            RootObject = 0;
            throw SCAGException("Semantic Analyser: Structure cannot has 2 rule objects");
        }


        Rule * rule = new Rule();
        try
        {
            PropertyObject propertyObject;
            rule->init(params,propertyObject);
        } catch(SCAGException& e)
        {
            if (NewObj) delete NewObj;
            delete rule;
            RootObject = 0;
            rule = 0;

            throw RuleEngineException(nLine, "Invalid object '%s' to create: %s",name.c_str(),e.what());
        }
        CurrentObject = rule;
        RootObject = rule;
    }
}

void SemanticAnalyser::DeliverEndTag(const std::string& name)
{
    if (CurrentObject) 
    {
        if (CurrentObject->FinishXMLSubSection(name))
        {
            CurrentObject = 0;
            if (HistoryObjectStack.size() > 0) 
            {
                CurrentObject = HistoryObjectStack.top();
                HistoryObjectStack.pop();
            }
        }
    }
}


Rule * XMLBasicHandler::ReturnFinalObject()
{
    Rule * rule = 0;

    if (CanReturnFinalObject) rule = analyser.ReturnRuleObject();
    return rule;
}



/////////////////////////////////////////////XMLBasicHandler/////////////////////////////

XMLBasicHandler::XMLBasicHandler(const ActionFactory& obj) : 
    analyser(obj),CanReturnFinalObject(false),logger(0)

{
    logger = Logger::getInstance("scag.re");
}

XMLBasicHandler::~XMLBasicHandler()
{
}

void XMLBasicHandler::characters(const XMLCh* const chars, const unsigned int length)
{
    // empty
}


void XMLBasicHandler::startElement(const XMLCh* const qname, AttributeList& attributes)
{
    StrX XMLQName(qname);
    SectionParams attr;

    unsigned int len = attributes.getLength();
    for (unsigned int index = 0; index < len; index++)
    {
        XMLCh const * XMLValue = attributes.getValue(index);
        XMLCh const * XMLName = attributes.getName(index);

        std::string value, name;

        Convertor::UCS2ToUTF8(XMLValue, XMLString::stringLen(XMLValue), value);
        Convertor::UCS2ToUTF8(XMLName, XMLString::stringLen(XMLName), name);

        attr[name.c_str()] = value;
    }

    analyser.DeliverBeginTag(XMLQName.localForm(), attr, m_pLocator->getLineNumber());
}


void XMLBasicHandler::endElement(const XMLCh* const qname)
{
    StrX XMLQName(qname);
    analyser.DeliverEndTag(XMLQName.localForm());
}

void XMLBasicHandler::endDocument()
{
    CanReturnFinalObject = true;
}

void XMLBasicHandler::setDocumentLocator(const Locator * const locator)
{
    m_pLocator = locator;
}


// ---------------------------------------------------------------------------
//  SAX2PrintHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void XMLBasicHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    //smsc_log_error(logger,"Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
    throw SCAGException("Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLBasicHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    //smsc_log_error(logger,"Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
    throw SCAGException("Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());

}

void XMLBasicHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

}}
