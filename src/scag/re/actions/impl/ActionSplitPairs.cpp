#include "ActionSplitPairs.h"
#include "scag/re/base/CommandAdapter2.h"
#include "core/buffers/Hash.hpp"

namespace scag2 {
namespace re {
namespace actions {

using smsc::core::buffers::Hash;

void ActionSplitPairs::init(const SectionParams &params, PropertyObject propertyObject) {
    ActionPairsBase::init(params,propertyObject);
    source_.init(params,propertyObject);
    rest_.init(params,propertyObject);
}

bool ActionSplitPairs::run(ActionContext &context) {
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
    Pair pair;
    Hash<std::string> pairsHash;
    while ( delPos != std::string::npos ) {
        delPos = source.find(pairDel, pos);
        string pairStr(source, pos, delPos-pos);
        smsc_log_debug(logger, "pair: '%s'", pairStr.c_str());
        pair = splitPair(pairStr, nameDel);
        pairsHash.Insert(pair.name.c_str(), pair.value);
        pos = delPos + pairDel.size();
    }

    for ( std::vector< ActionPair* >::const_iterator i = pairActions_.begin(); i != pairActions_.end(); ++i) {
        const char* name = (*i)->getName(context);
        Property *valProp = (*i)->getValueProperty(context);
        if (!name || !valProp) { 
            continue;
        }
        const string *val = pairsHash.GetPtr(name);
        if (val) {
            valProp->setStr((*val).c_str());
            pairsHash.Delete(name);
        }
    }

    Property *restProp = rest_.getProperty(context);
    if (!restProp) {
        return true;
    }

    string rest = "";
    Hash<std::string>::Iterator it = pairsHash.getIterator();
    std::string* value;
    char *name = 0;
    while(it.Next(name, value)) {
        rest.append(name);
        rest.append(nameDel);
        rest.append(*value);
        rest.append(pairDel);
    }
    restProp->setStr(rest.c_str());
    return true;
}

}}}

