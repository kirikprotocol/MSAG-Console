#ifndef __XMLHANDLERS2_H__
#define __XMLHANDLERS2_H__

#include <string>
#include <stack>


#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <util/Exception.hpp>


#include <core/buffers/Array.hpp>
#include <acls/interfaces.h>
#include "Rule2.h"


namespace scag2 {
namespace re { 

using namespace smsc::util;
using smsc::core::buffers::Hash;

class IParserHandler;
class ActionFactory;


XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END


class XMLBasicHandler;
class Rule;

class SemanticAnalyser
{
    std::stack <IParserHandler *> HistoryObjectStack;
    IParserHandler * CurrentObject;
    const ActionFactory * factory;
    Rule * RootObject;
public:
    void DeliverBeginTag(const std::string& name,const SectionParams& params,int nLine);
    void DeliverEndTag(const std::string& name);

    Rule * ReturnRuleObject(); 

    SemanticAnalyser(const ActionFactory& obj);
    ~SemanticAnalyser();
};


class XMLBasicHandler : public HandlerBase
{
    SemanticAnalyser analyser;
    bool CanReturnFinalObject;
    const Locator * m_pLocator;
    Logger * logger;

    void characters(const XMLCh* const chars, const unsigned int length);

protected:
public:
    Rule * ReturnFinalObject();

    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    XMLBasicHandler(const ActionFactory& obj);
    ~XMLBasicHandler();

    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);
    void SetIParserHandler(IParserHandler * obj);
    virtual void setDocumentLocator(const Locator * const locator);

    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
    void endDocument();
};

class StrX
{
public :
    StrX(const XMLCh* const toTranscode) { fLocalForm = XMLString::transcode(toTranscode);  }

    ~StrX() { XMLString::release(&fLocalForm); }

    const char* localForm() const { return fLocalForm; }

private :
    char*   fLocalForm;
};

}}

#endif
