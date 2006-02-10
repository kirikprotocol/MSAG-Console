#ifndef __XMLHANDLERS_H__
#define __XMLHANDLERS_H__

#include <string>
#include <stack>


#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

#include <util/Exception.hpp>


#include <core/buffers/Array.hpp>
#include <acls/interfaces.h>
#include "scag/re/Rule.h"


namespace scag { namespace re { 

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


class XMLBasicHandler : public HandlerBase, private XMLFormatTarget
{
    SemanticAnalyser analyser;
    bool CanReturnFinalObject;
    const Locator * m_pLocator;
    Logger * logger;
    XMLFormatter  fFormatter;

    void writeChars(const XMLByte* const toWrite);
    void writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter);
    void characters(const XMLCh* const chars, const unsigned int length);

protected:
public:
    Rule * ReturnFinalObject();

    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    XMLBasicHandler(const ActionFactory& obj, const char* const encodingName);
    ~XMLBasicHandler();

    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);
    void SetIParserHandler(IParserHandler * obj);
    virtual void setDocumentLocator(const Locator * const locator);



    // -----------------------------------------------------------------------
    //  Implementations of the SAX ErrorHandler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
    void endDocument();
};
/*
class XMLBasicTranscoder : public XMLTranscoder
{
public:

};  */

class StrX
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    StrX(const XMLCh* const toTranscode)
    {
        // Call the private transcoding method
        fLocalForm = XMLString::transcode(toTranscode);
    }

    ~StrX()
    {
        XMLString::release(&fLocalForm);
    }

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const char* localForm() const
    {
        return fLocalForm;
    }

private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fLocalForm
    //      This is the local code page form of the string.
    // -----------------------------------------------------------------------
    char*   fLocalForm;
};
/*
inline XERCES_STD_QUALIFIER ostream& operator<<(XERCES_STD_QUALIFIER ostream& target, const StrX& toDump)
{
    //target << toDump.localForm();
    return target;
}        */


}}

#endif

