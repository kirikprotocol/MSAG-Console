#include "ActionFormatPairs.h"
#include "scag/re/base/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionFormatPairs::init(const SectionParams &params, PropertyObject propertyObject) {
    ActionPairsBase::init(params,propertyObject);
    prefix_.init(params,propertyObject);
    postfix_.init(params,propertyObject);
    result_.init(params,propertyObject);
}

bool ActionFormatPairs::run(ActionContext &context) {
    smsc_log_debug(logger, "run action: %s", opname());
    std::string result = prefix_.getValue(context);
    std::string nameDel = nameDel_.getValue(context);
    std::string pairDel = pairDel_.getValue(context);
    Property *p = 0;
    for (std::vector< ActionPair* >::const_iterator i = pairActions_.begin(); i != pairActions_.end(); ++i) {
        result.append( (*i)->getName(context) );
        result += nameDel;
        result.append( (*i)->getValue(context) );
        result += pairDel;
    }
    result += postfix_.getValue(context);
    Property *resultProperty = result_.getProperty(context);
    if (resultProperty) {
        resultProperty->setStr(result.c_str());
    }
    return true;
}

}}}

