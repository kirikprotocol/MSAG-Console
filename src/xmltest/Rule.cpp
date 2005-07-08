#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include "Rule.h"
#include "SAX2Print.hpp"


using namespace std;
using namespace smsc::util;

/*
#ifndef MEMPARSE_ENCODING
   #if defined(OS390)
      #define MEMPARSE_ENCODING "ibm-1047-s390"
   #elif defined(OS400)
      #define MEMPARSE_ENCODING "ibm037"
   #else
      #define MEMPARSE_ENCODING "ascii"
   #endif
#endif  ifndef MEMPARSE_ENCODING */
/*
XMLRuleHandler::XMLRuleHandler(const std::string SectionName)
{

}
  
*/


Rule::Rule(const SectionParams& params)
{
}

Rule::~Rule()
{
    int key;
    EventHandler * value;

    for (IntHash <EventHandler *>::Iterator it = Handlers.First(); it.Next(key, value);)
    {
        delete value;
    }

    cout << "Rule released" << endl;
}


//////////////IParserHandler Interfase///////////////////////

bool Rule::SetChildObject(const IParserHandler * child)
{
    if (!child) return false;

    IParserHandler * _child = const_cast<IParserHandler *>(child);
    EventHandler * eh = dynamic_cast<EventHandler *>(_child);
    Handlers.Insert(Handlers.Count(),eh);
    return true;
}

//////////////IParserHandler Interfase///////////////////////


