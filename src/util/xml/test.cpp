#include <iostream>
#include "util/xml/DOMTreeReader.h"
#include "logger/Logger.h"
#include "util/xml/utilFunctions.h"
#include "util/xml/SmscTranscoder.h"
#include "util/xml/init.h"
#include <xercesc/util/TransENameMap.hpp>
#include <xercesc/internal/IANAEncodings.hpp>

using namespace std;
using namespace smsc::util::xml;

ostream & operator << (ostream & out, const DOMElement & elem);

int usage(char * name);

int main(int argc, char ** argv)
{
  if (argc != 2)
    return usage(argv[0]);
  
  smsc::logger::Logger::Init();
  initXerces();
  {
    for (int i=0; i<1024; i++) {
      cout << i << " --------------------------------------------------------" << endl;
      DOMTreeReader reader;
      DOMDocument *doc = reader.read(argv[1]);
      DOMElement *docElem = doc->getDocumentElement();
      if (docElem)
        cout << "docElem readed" << endl; //*docElem;
      else 
        cerr << "docElem is null" << endl;
    }
    cout << "finished" << endl;
  }
  abort();
  TerminateXerces();
}

int usage(char * name)
{
  cerr << name << " <xml filename> "<< endl;
  return 1;
}

ostream & operator << (ostream & out, const DOMElement & elem)
{
  out << '<' << XmlStr(elem.getNodeName());
  DOMNamedNodeMap *attrs = elem.getAttributes();
  for (XMLSize_t i = 0; i<attrs->getLength(); i++)
  {
    DOMNode *item = attrs->item(i);
    out << " " << XmlStr(item->getNodeName()) << "=\"" << XmlStr(item->getNodeValue()) << '"';
  }
  out << '>' << endl;
  DOMNodeList *list = elem.getChildNodes();
  for (XMLSize_t i = 0; i<list->getLength(); i++)
  {
    DOMNode *item = list->item(i);
    if (item->getNodeType() == DOMNode::ELEMENT_NODE)
      out << *(DOMElement*)item;
  }
  out << "</" << XmlStr(elem.getNodeName()) << '>' << endl;
  return out;
}
