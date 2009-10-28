#include "ActionPairsBase.h"
#include "scag/util/PtrDestroy.h"
#include "scag/re/base/ActionFactory2.h"
#include "scag/re/base/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {

ActionPairsBase::~ActionPairsBase() {
    std::for_each( pairActions_.begin(), pairActions_.end(), PtrDestroy() );
}

void ActionPairsBase::init(const SectionParams &params, PropertyObject propertyObject) {
    nameDel_.init(params,propertyObject);
    pairDel_.init(params,propertyObject);
    pobj_ = propertyObject;
}

IParserHandler * ActionPairsBase::StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory){
    smsc_log_debug(logger, "%s: StartXMLSubSection %s", opname(), name.c_str());
    std::auto_ptr<ActionPair> act( new ActionPair(readonlyPairName_, readonlyPairValue_) );
    if ( !act.get() ) {
        throw SCAGException("action %s was not created", name.c_str());
    }
    act->init(params, pobj_);
    pairActions_.push_back(act.release());
    return NULL;
}

bool ActionPairsBase::FinishXMLSubSection(const std::string &name) {
    smsc_log_debug(logger, "%s: FinishXMLSubsection %s", opname(), name.c_str());
    return (name == opname());
}

Pair ActionPairsBase::splitPair(const std::string& pair, const std::string& nameDel) {
	if (pair.empty()) {
		return Pair();
	}
    size_t delPos = pair.find(nameDel);
	if (nameDel.empty() || delPos == std::string::npos) {
		return Pair(pair, "");
	}
	size_t valPos = delPos + nameDel.size();
	Pair nameVal(std::string(pair.begin(), pair.begin() + delPos),
			valPos >= pair.size() ? std::string("") : std::string(pair.begin() + valPos, pair.end()));
    smsc_log_debug(logger, "name='%s' val='%s'", nameVal.name.c_str(), nameVal.value.c_str());
    return nameVal;
}

}}}

