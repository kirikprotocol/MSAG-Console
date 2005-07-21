#ifndef __SCAG_RULE_ENGINE_ACTION__
#define __SCAG_RULE_ENGINE_ACTION__

#include "ActionContext.h"
#include <scag/re/actions/IParserHandler.h>
#include <core/buffers/IntHash.hpp>

#include <iostream>

using smsc::core::buffers::IntHash;
using namespace std;


namespace scag { namespace re { namespace actions 
{

class Action : public IParserHandler
{
public:
    virtual bool run(ActionContext& context) = 0;
    virtual void init(const SectionParams& params) = 0;
};

}}}

#endif // __SCAG_RULE_ENGINE_ACTION__

