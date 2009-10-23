#include "ActionParsePairs.h"
#include "scag/re/base/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionParsePairs::init(const SectionParams &params, PropertyObject propertyObject) {
    ActionPairsBase::init(params,propertyObject);
    source_.init(params,propertyObject);
    rest_.init(params,propertyObject);
}

bool ActionParsePairs::run(ActionContext &context) {
    smsc_log_debug(logger, "run action: %s", opname());
    std::string source = source_.getValue(context);
    std::string pairDel = pairDel_.getValue(context);
    std::string nameDel = nameDel_.getValue(context);
    smsc_log_debug(logger, "source string: '%s'", source.c_str());
    if (source.empty() || pairDel.empty()) {
        return true;
    }
   	size_t pos = 0;
	size_t delPos = 0;
    size_t pairDelSize = pairDel.size();
    for ( std::vector< ActionPair* >::const_iterator i = pairActions_.begin(); i != pairActions_.end() && delPos != std::string::npos; ++i) {
		delPos = source.find(pairDel, pos);
		string pair(source, pos, delPos-pos);
        smsc_log_debug(logger, "pair: '%s'", pair.c_str());
        (*i)->setPair(splitPair(pair, nameDel), context);
        pos = delPos + pairDel.size();
    }
    Property *restProp = rest_.getProperty(context);
    if (restProp && pos != std::string::npos) {
        string rest(source, pos, source.size() - pos);
        restProp->setStr(rest.c_str());
    }
    return true;
}

}}}

