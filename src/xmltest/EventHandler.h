#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#include <iostream>
#include <core/buffers/IntHash.hpp>

#include "XMLHandlers.h"
#include "util/Exception.hpp"
#include "IParserHandler.h"
#include "Action.h"

using namespace smsc::core::buffers;

class Action;

class EventHandler : public IParserHandler
{
    EventHandler(const EventHandler &);
    IntHash<Action *> actions;
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual bool SetChildObject(const IParserHandler * child);
//////////////IParserHandler Interfase///////////////////////
public:
    EventHandler(const SectionParams& params);
    ~EventHandler();

};

#endif
