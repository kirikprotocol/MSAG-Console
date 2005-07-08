#include "XMLHandlers.h"
#include <iostream.h>



#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

#include "Rule.h"

static const char*  encodingName    = "LATIN1";
static XMLFormatter::UnRepFlags unRepFlags  = XMLFormatter::UnRep_CharRef;
static const XMLCh  gEndElement[] = { chOpenAngle, chForwardSlash, chNull };

using namespace smsc::util;


/////////////////////////////////////////////SemanticAnalyser/////////////////////////////


SemanticAnalyser::SemanticAnalyser(const ActionFactory& obj)
{
    factory = &obj;
    CurrentObject = 0;

    ValidSimpleTags["rule"] = tgRuleSection;
    ValidSimpleTags["handler"] = tgEventHandlerSection;

    factory->FillTagHash(ValidSimpleTags);

/*    
    ValidSimpleTags["set"] = tgActionSection;
    ValidSimpleTags["if"] = tgActionSection;
    ValidSimpleTags["choose"] = tgActionSection;
    */
}

SemanticAnalyser::~SemanticAnalyser()
{
    for (vector <IParserHandler *>::iterator it = HistoryObjectStack.begin();it!=HistoryObjectStack.end();it++)
    {
        delete (*it);
    }
    if (CurrentObject) delete CurrentObject;
}

IParserHandler * SemanticAnalyser::ReturnCurrentObject()
{
    IParserHandler * RObj = CurrentObject;
    CurrentObject = 0;
    return RObj;
}

void SemanticAnalyser::DeliverBeginTag(const std::string& name,const SectionParams& params)
{
    if (isValidSimpleTag(name)) StartCreateObject(name,params);
    else StartFillObject(name,params);
}

void SemanticAnalyser::DeliverEndTag(const std::string& name)
{
    if (isValidSimpleTag(name)) FinishCreateObject(name);
    else FinishFillObject(name);
}


void SemanticAnalyser::StartCreateObject(const std::string& name,const SectionParams& params)
{
    if (CurrentObject) HistoryObjectStack.push_back(CurrentObject);

    switch (ValidSimpleTags[name.data()])  
    {
    case tgRuleSection: CurrentObject = new Rule(params); break;
    case tgEventHandlerSection: CurrentObject = new EventHandler(params); break;
    case tgActionSection: CurrentObject = factory->CreateAction(name,params); break;
    }
}
                                                         
void SemanticAnalyser::FinishCreateObject(const std::string& name)
{
    IParserHandler * ParentObject = 0;
    if (HistoryObjectStack.size() > 0) ParentObject = HistoryObjectStack.back();

    if (ParentObject) 
    {
        HistoryObjectStack.pop_back();
        if (!ParentObject->SetChildObject(CurrentObject)) delete CurrentObject;
        CurrentObject = ParentObject;
    }
}

void SemanticAnalyser::StartFillObject(const std::string& name,const SectionParams& params)
{
    if (CurrentObject) CurrentObject->StartXMLSubSection(name,params);
}

void SemanticAnalyser::FinishFillObject(const std::string& name)
{
    if (CurrentObject) CurrentObject->FinishXMLSubSection(name);
}


/////////////////////////////////////////////XMLBasicHandler/////////////////////////////

XMLBasicHandler::XMLBasicHandler(const ActionFactory& obj) : analyser(obj)
{
    std::cout << "Constructor HANDLER " << endl;
    CanReturnFinalObject = false;
}

XMLBasicHandler::~XMLBasicHandler()
{
}

IParserHandler * XMLBasicHandler::ReturnFinalObject()
{
    IParserHandler * RObj = 0;

    if (CanReturnFinalObject) RObj = analyser.ReturnCurrentObject();
    return RObj;
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
    cout << "end document" << endl;
    CanReturnFinalObject = true;
}


// ---------------------------------------------------------------------------
//  SAX2PrintHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void XMLBasicHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    cout << "\nError at file " << fname.localForm()
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "\n  Message: " << msg.localForm() << XERCES_STD_QUALIFIER endl;
}

void XMLBasicHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());

    cout << "\nFatal Error at file " << fname.localForm()
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "\n  Message: " << msg.localForm() << XERCES_STD_QUALIFIER endl;
}

void XMLBasicHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());

    cout << "\nWarning at file " << fname.localForm()
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "\n  Message: " << msg.localForm() << XERCES_STD_QUALIFIER endl;
}


