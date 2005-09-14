#include <scag/re/XMLHandlers.h>

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include "scag/SAX2Print.hpp"

namespace scag { namespace re { 

static const char*  encodingName    = "LATIN1";
static XMLFormatter::UnRepFlags unRepFlags  = XMLFormatter::UnRep_CharRef;
static const XMLCh  gEndElement[] = { chOpenAngle, chForwardSlash, chNull };

using namespace smsc::util;


/////////////////////////////////////////////SemanticAnalyser/////////////////////////////


SemanticAnalyser::SemanticAnalyser(const ActionFactory& obj) : factory(&obj),CurrentObject(0),RootObject(0)
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

void SemanticAnalyser::DeliverBeginTag(const std::string& name,const SectionParams& params)
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
            throw SCAGException("Semantic Analyser: Invalid object '%s' to create: %s",name.c_str(),e.what()); 
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
            throw SCAGException("Semantic Analyser: Invalid object '%s' to create: %s",name.c_str(),e.what()); 
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





/////////////////////////////////////////////XMLBasicHandler/////////////////////////////

XMLBasicHandler::XMLBasicHandler(const ActionFactory& obj) : analyser(obj),CanReturnFinalObject(false)
{
}

XMLBasicHandler::~XMLBasicHandler()
{
}

Rule * XMLBasicHandler::ReturnFinalObject()
{
    Rule * rule = 0;

    if (CanReturnFinalObject) rule = analyser.ReturnRuleObject();
    return rule;
}


void XMLBasicHandler::startElement(const XMLCh* const qname, AttributeList& attributes)
{
    StrX XMLQName(qname);
    SectionParams attr;

    unsigned int len = attributes.getLength();
    for (unsigned int index = 0; index < len; index++)
    {
        StrX * AttrName = new StrX(attributes.getName(index));
        StrX * AttrValue = new StrX(attributes.getValue(index));

        attr[AttrName->localForm()] = AttrValue->localForm();

        delete AttrName;
        delete AttrValue;
    }

    analyser.DeliverBeginTag(XMLQName.localForm(),attr);
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


// ---------------------------------------------------------------------------
//  SAX2PrintHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void XMLBasicHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void XMLBasicHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());

}

void XMLBasicHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger,"Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

}}

