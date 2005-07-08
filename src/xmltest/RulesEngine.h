#ifndef _RULESENGINE_H_
#define _RULESENGINE_H_


#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <iostream>
#include <core/buffers/Hash.hpp>

#include "SAX2Print.hpp"
#include "XMLHandlers.h"
#include "Rule.h"
#include "Action.h"
#include "util/Exception.hpp"

using namespace smsc::core::buffers;


class ActionFactory
{
public: 
    Action * CreateAction(const std::string& name, const SectionParams& params) const;
    void FillTagHash(smsc::core::buffers::Hash<int>& TagHash) const;
};

class Rule;

class RulesEngine
{
    ActionFactory factory;
    IntHash<Rule *> Rules;
public:
    int ParseFile(const std::string& xmlFile);

    RulesEngine();
    ~RulesEngine();
};

#endif
