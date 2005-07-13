#ifndef __XMLHANDLERS_H__
#define __XMLHANDLERS_H__

#include <stdio.h>
#include <string>

#include <xercesc/sax/HandlerBase.hpp>
#include <core/buffers/Hash.hpp>
#include <util/Exception.hpp>


#include <core/buffers/Array.hpp>
#include <acls/interfaces.h>
//#include "scag/re/RuleEngine.h"
#include "scag/re/actions/IParserHandler.h"
#include "scag/re/Rule.h"

namespace scag { namespace re { 

using namespace smsc::util;
using smsc::core::buffers::Hash;
using namespace std;

class IParserHandler;
class ActionFactory;


XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

typedef smsc::core::buffers::Hash<std::string> SectionParams;

enum TagsID
{
    tgRuleSection,
    tgEventHandlerSection,
    tgActionSection
};


class XMLBasicHandler;

class SemanticAnalyser
{
    smsc::core::buffers::Hash<int> ValidSimpleTags;
    vector <IParserHandler *> HistoryObjectStack;
    IParserHandler * CurrentObject;
    const ActionFactory * factory;

    void StartCreateObject(const std::string& name,const SectionParams& params);
    void FinishCreateObject(const std::string& name);
    void StartFillObject(const std::string& name,const SectionParams& params);
    void FinishFillObject(const std::string& name);

    bool isValidSimpleTag(const std::string& name) {return ValidSimpleTags.Exists(name.data());}
    
public:
    void DeliverBeginTag(const std::string& name,const SectionParams& params);
    void DeliverEndTag(const std::string& name);

    IParserHandler * ReturnCurrentObject(); //return object and deligate ownership

    SemanticAnalyser(const ActionFactory& obj);
    ~SemanticAnalyser();
};


class XMLBasicHandler : public HandlerBase
{
    SemanticAnalyser analyser;
    bool CanReturnFinalObject;
protected:
public:
    IParserHandler * ReturnFinalObject();

    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    XMLBasicHandler(const ActionFactory& obj);
    ~XMLBasicHandler();

    void startElement(const XMLCh* const qname, AttributeList& attributes);
    void endElement(const XMLCh* const qname);
    void SetIParserHandler(IParserHandler * obj);

    // -----------------------------------------------------------------------
    //  Implementations of the SAX ErrorHandler interface
    // -----------------------------------------------------------------------
    void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
    void endDocument();
};


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

}}

#endif

