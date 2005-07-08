#ifndef __RULE_H__
#define __RULE_H__


#include <memory>
#include "util/Exception.hpp"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <core/buffers/Hash.hpp>


#include "EventHandler.h"
#include "IParserHandler.h"

class EventHandler;
 
class Rule : public IParserHandler
{
    Rule(const Rule &);
    IntHash <EventHandler *> Handlers;
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual bool SetChildObject(const IParserHandler * child);
//////////////IParserHandler Interfase///////////////////////
public:
    Rule(const SectionParams& params);
    ~Rule();
};

#endif
