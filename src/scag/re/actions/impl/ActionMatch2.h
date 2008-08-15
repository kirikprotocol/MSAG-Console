#ifndef __SCAG_RULE_ENGINE_ACTION_MATCH2__
#define __SCAG_RULE_ENGINE_ACTION_MATCH2__

#include "scag/re/base/ActionContext2.h"
#include "scag/re/base/Action2.h"
#include "util/regexp/RegExp.hpp"


namespace scag2 {
namespace re {
namespace actions {

using namespace smsc::util::regexp;

class ActionMatch : public Action
{
    ActionMatch(const ActionMatch&);

    std::string strRegexp;
    std::string wstrRegexp;

    std::string strValue;
    std::string wstrValue;

    FieldType m_ftValue, ftRegexp;

    std::string strResult;
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionMatch () {};

    virtual ~ActionMatch() {};

};


}}}


#endif

