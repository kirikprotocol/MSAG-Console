#include <scag/re/XMLHandlers.h>

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/sax/Locator.hpp>
#include <xercesc/util/TransService.hpp>

namespace scag { namespace re { 

//static const char*  encodingName    = "LATIN1";
static XMLFormatter::UnRepFlags unRepFlags  = XMLFormatter::UnRep_CharRef;
static XMLTranscoder::UnRepOpts unRepOpts = XMLTranscoder::UnRep_Throw;

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

XMLBasicHandler::XMLBasicHandler(const ActionFactory& obj,const char* const encodingName) : 
    analyser(obj),CanReturnFinalObject(false),logger(0),
    fFormatter(encodingName, 0, this, XMLFormatter::NoEscapes)

{
    logger = Logger::getInstance("scag.re");
}

XMLBasicHandler::~XMLBasicHandler()
{
}


void XMLBasicHandler::writeChars(const XMLByte* const toWrite)
{
}

void XMLBasicHandler::writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter)
{
//    XERCES_STD_QUALIFIER cout.write((char *) toWrite, (int) count);
//    XERCES_STD_QUALIFIER cout.flush();
}


void XMLBasicHandler::characters(const XMLCh* const chars, const unsigned int length)
{
    //fFormatter.formatBuf(chars, length, XMLFormatter::CharEscapes);
}


void XMLBasicHandler::startElement(const XMLCh* const qname, AttributeList& attributes)
{
    StrX XMLQName(qname);
    SectionParams attr;
    XMLByte buff[10000];

    unsigned int len = attributes.getLength();
    for (unsigned int index = 0; index < len; index++)
    {
        //StrX * AttrName = new StrX(attributes.getName(index));
        //StrX * AttrValue = new StrX(attributes.getValue(index));

       /* fFormatter  << XMLFormatter::NoEscapes
                    << chSpace << attributes.getName(index)
                    << chEqual << chDoubleQuote
                    << XMLFormatter::AttrEscapes
                    << attributes.getValue(index)
                    << XMLFormatter::NoEscapes
                    << chDoubleQuote;*/

        const XMLTranscoder * transcoder = fFormatter.getTranscoder();

        unsigned int charsEaten;

        XMLTranscoder * _transcoder = const_cast <XMLTranscoder *>(transcoder);

        XMLCh const * XMLValue = attributes.getValue(index);
        XMLCh const * XMLName = attributes.getName(index);

        std::wstring value;

        //value.append((wchar_t *)XMLValue, XMLString::stringLen(XMLValue));

        const char * XMLPtr = (char *)XMLValue;

        char temp[1024];

        for(int j=0; j < XMLString::stringLen(XMLValue); j++) 
        {
            temp[j*4] = 0;
            temp[j*4+1] = 0;
            temp[j*4+2] = XMLPtr[j*2];
            temp[j*4+3] = XMLPtr[j*2+1];
        }
        value.append((wchar_t*)temp,XMLString::stringLen(XMLValue));

        const char * aa = (char *)value.c_str();
        /*
        if (XMLString::stringLen(XMLValue) == 14)
        {
            std::string str =  FormatWStr(value);
            value = UnformatWStr(str);

            for (int j=0; j < 14;j++) 
            {
                char chr;
                wctomb(&chr,value[j]);

                //std::cout << "?" << (int)chr << std::endl;
                std::cout << "--" << (int)XMLPtr[j*2] << "--" << (int)XMLPtr[j*2+1] << std::endl;
            }

        }   */
  
        _transcoder->transcodeTo(XMLName, XMLString::stringLen(XMLName), buff, 1024, charsEaten,unRepOpts);

        attr[(char *)buff] = value;
    }

    analyser.DeliverBeginTag(XMLQName.localForm(),attr,m_pLocator->getLineNumber());

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

