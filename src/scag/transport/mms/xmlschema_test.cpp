#include "logger/Logger.h"
#include "sms/sms.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <string.h>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMWriter.hpp>

#include "MmsParser.h"

#include <scag/util/encodings/Encodings.h>

#include "XMLHandlers.h"
#include "MmsMsg.h"
#include "MmsFactory.h"

#define MEMPARSE_ENCODING "UTF-8"

XERCES_CPP_NAMESPACE_USE

//using namespace scag::transport::mms;

class A {
  scag::transport::mms::MmsMsg m;
};

class B {
public:
  B() { __trace__("create B"); }  
  ~B() { __trace__("delete B"); }  
};
class C {
public:
  B* b;
  C() { __trace__("create C"); }  
  ~C() { if (b) delete b; __trace__("delete C"); }  
  void createB() { b = new B(); };
  B* getB() { if (b) return b; else return 0; };
};

class StrX {
public:
  StrX(const XMLCh* const toTranscode) {
    fLocalForm = XMLString::transcode(toTranscode);
  }
  ~StrX() {
    XMLString::release(&fLocalForm);
  }
  const char* localForm() const {
    return fLocalForm;
  }
private:
  char* fLocalForm;
};

class MyErrorHandler: public HandlerBase {
  std::string tag_name;
public:
  MyErrorHandler():fSawErrors(false) {};
  ~MyErrorHandler() {};
  bool getSawErrors() const {
    return fSawErrors;
  }
  void characters(const XMLCh* const ch, const unsigned int length) {
    StrX strx(ch);
    __trace2__("TEXT : %s", strx.localForm());
  }
  void startElement(const XMLCh* const name, AttributeList& attributs) {
    StrX strx(name);
    tag_name = strx.localForm();
    __trace2__("Start Element : %s", tag_name.c_str());
    size_t len = attributs.getLength();
    for (int i = 0; i < len; ++i) {
      XMLCh const * xml_name = attributs.getName(i);
      XMLCh const * xml_value = attributs.getValue(i);
      std::string value, name;
      scag::util::encodings::Convertor::UCS2ToUTF8(xml_name, XMLString::stringLen(xml_name), name);
      scag::util::encodings::Convertor::UCS2ToUTF8(xml_value, XMLString::stringLen(xml_value), value);
      __trace2__("Attribute Name=\'%s\' Value=\'%s\'", name.c_str(), value.c_str());
    }
  }
  void endElement(const XMLCh* const name) {
    StrX strx(name);
    __trace2__("End Element : %s", strx.localForm());
  }    
  void warning(const SAXParseException& e) {
    StrX strx(e.getMessage());
    __trace2__("XML Warning : %s", strx.localForm());
  }
  void error(const SAXParseException& e) {
    fSawErrors = true;
    StrX system_id(e.getSystemId());
    StrX msg(e.getMessage());
    __trace2__("XML Error in file \"%s\" line:%d, column:%d \nMESSAGE : %s", 
               system_id.localForm(), e.getLineNumber(), e.getColumnNumber(), msg.localForm());
  }
  void fatalError(const SAXParseException& e) {
    fSawErrors = true;
    StrX system_id(e.getSystemId());
    StrX msg(e.getMessage());
    __trace2__("Fatal XML Error in file \"%s\" line:%d, column:%d \nMESSAGE : %s", 
               system_id.localForm(), e.getLineNumber(), e.getColumnNumber(), msg.localForm());
  }
  void resetErrors() {
    fSawErrors = false;
  }
private:
  bool fSawErrors;
};

int main(int argc, const char* argv[]) {
  smsc::logger::Logger::Init();
  try {
    XMLPlatformUtils::Initialize();
  } catch (const XMLException& toCatch) {
    StrX strx(toCatch.getMessage());
    __trace2__("Error during XML Initialization! Message:", strx.localForm());
    return 0;
  } 
  std::ifstream f(argv[1]);
  if (!f) {
    __trace__("Can't open file");
    return 0;
  }

  static const char* xml_msg_id = "submit request";
  const char* xml_buf = "<root> <sub_tag>hellow</sub_tag> ";

  std::string xml_msg;
  getline(f, xml_msg, static_cast<char>(EOF));
  
  std::cout << xml_msg << std::endl;
  
  MemBufInputSource* memBufIS = new MemBufInputSource
  (
    (const XMLByte*) xml_msg.c_str(),
    strlen(xml_msg.c_str()),
    //(const XMLByte*) xml_buf,
    //strlen(xml_buf),
    argv[1],
    true
  );
  
  //XercesDOMParser* parser = new XercesDOMParser;
  SAXParser* parser = new SAXParser;
  parser->setValidationScheme(SAXParser::Val_Always);
  parser->setDoNamespaces(true);
  parser->setDoSchema(true);
  parser->setValidationSchemaFullChecking(true);
  parser->setExternalSchemaLocation("http://www.3gpp.org/ftp/Specs/archive/23_series/23.140/schema/REL-6-MM7-1-4 ./REL-6-MM7-1-4.xsd");
  //parser->setExternalSchemaLocation("http://schemas.xmlsoap.org/soap/envelope/ ./envelope.xsd");
  MyErrorHandler *err_handler = new MyErrorHandler;
  scag::transport::mms::MmsFactory factory;
  scag::transport::mms::XMLHandler handler(factory);
  parser->setErrorHandler(&handler);
  parser->setDocumentHandler(&handler);
  try {
    parser->parse(*memBufIS);
  }
  catch (const OutOfMemoryException& e) {
    StrX strx(e.getMessage());
    __trace2__("Out of Memory EXCEPTION! :", strx.localForm());
    //return 0;
  }
  catch (const XMLException& e) {
    StrX strx(e.getMessage());
    __trace2__("XML EXCEPTION! :", strx.localForm());
    //return 0;
  }
  catch (const DOMException& e) {
    //StrX strx(e.getMessage());
    __trace2__("DOM EXCEPTION! %d", e.code);
    //return 0;
  }
  catch (...) {
    std::cout << "EXCEPTION!!!" << std::endl;
  }
  scag::transport::mms::MmsMsg* mms_msg = handler.getMmsMsg();
  __trace__("*******************************************");
  if (mms_msg)
    mms_msg->test();
    std::string serialized_msg = mms_msg->serialize();
    std::ofstream out("result.xml");
    if (out) {
      out << serialized_msg;
    }
    //__trace2__("%s", serialized_msg.c_str());
  __trace__("*******************************************");
  delete memBufIS; 
  delete parser;
  delete err_handler;
  /*
  scag::transport::mms::HttpMsg http_msg;
  http_msg.setFirstLine("POST /mms/mm7 HTTP/1.1");
  http_msg.setFirstLine("POST /mms/mm7 HTTP/1.1 NEW!");
  __trace2__("HTTP FIRST LINE:%s", http_msg.getFirstLine());
  int sn = 100;
  char sn_buf[100];
  int actn = snprintf(sn_buf, 99, "%d", sn);
  __trace2__("sn=%d buf =\'%s\' actn=%d", sn, sn_buf, actn); 
  actn = snprintf(sn_buf, 99, "%d", sn);
  __trace2__("sn=%d buf =\'%s\' actn=%d", sn, sn_buf, actn); 
  */
  return 1;
}