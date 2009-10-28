#include "ActionPair.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionPair::init(const SectionParams &params, PropertyObject propertyObject) {
    name_.init(params,propertyObject);
    value_.init(params,propertyObject);
}

void ActionPair::setPair(const Pair& pair, ActionContext &context) {
    Property* pname = name_.getProperty(context);
    if (pname) {
        pname->setStr(pair.name.c_str());
    }
    Property* pvalue = value_.getProperty(context);
    if (pvalue) {
        pvalue->setStr(pair.value.c_str());
    }
}

const char* ActionPair::getName(ActionContext &context) { 
    return name_.getValue(context);
}

const char* ActionPair::getValue(ActionContext &context) { 
    return value_.getValue(context);
}

}}}

