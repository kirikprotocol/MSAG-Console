/* $Id$ */

#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/TransService.hpp>

#include "XMLHandlers.h"

namespace scag { namespace bill { namespace infrastruct {

//static const char*  encodingName    = "LATIN1";
static XMLFormatter::UnRepFlags unRepFlags  = XMLFormatter::UnRep_CharRef;
static XMLTranscoder::UnRepOpts unRepOpts = XMLTranscoder::UnRep_Throw;

static const XMLCh  gEndElement[] = { chOpenAngle, chForwardSlash, chNull };

using namespace smsc::util;

/*void SemanticAnalyser::DeliverBeginTag(const std::string& name,const SectionParams& params,int nLine)
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
}*/



/////////////////////////////////////////////XMLBasicHandler/////////////////////////////

XMLBasicHandler::XMLBasicHandler(const char* const encodingName, IntHash<uint32_t> *h) : 
    logger(0), hash(h), fFormatter(encodingName, 0, this, XMLFormatter::NoEscapes)

{
    logger = Logger::getInstance("xmlhndlr");
}

XMLBasicHandler::~XMLBasicHandler()
{
}

void XMLBasicHandler::writeChars(const XMLByte* const toWrite)
{
}

void XMLBasicHandler::writeChars(const XMLByte* const toWrite, const unsigned int count, XMLFormatter* const formatter)
{
}

void XMLBasicHandler::characters(const XMLCh* const chars, const unsigned int length)
{
}

void XMLBasicHandler::startElement(const XMLCh* const qname, AttributeList& attributes)
{
    StrX XMLQName(qname);
//    XMLByte buff[1024];

	smsc_log_debug(logger, "element:<<%s>>", XMLQName.localForm());
    for (unsigned int index = 0; index < attributes.getLength(); index++)
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

        //const XMLTranscoder * transcoder = fFormatter.getTranscoder();

        unsigned int charsEaten;

//        XMLTranscoder * _transcoder = const_cast <XMLTranscoder *>(transcoder);

        XMLCh const * XMLValue = attributes.getValue(index);
        XMLCh const * XMLName = attributes.getName(index);

		smsc_log_debug(logger, "<<%s %s>>", XMLName, XMLValue);
//        std::string value;

        //value.append((wchar_t *)XMLValue, XMLString::stringLen(XMLValue));

    //    value.assign((char *)XMLValue,XMLString::stringLen(XMLValue)*2);

  //      _transcoder->transcodeTo(XMLName, XMLString::stringLen(XMLName), buff, 1024, charsEaten,unRepOpts);
    }

}


void XMLBasicHandler::endElement(const XMLCh* const qname)
{
    StrX XMLQName(qname);
}

void XMLBasicHandler::endDocument()
{
}

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

}}}

