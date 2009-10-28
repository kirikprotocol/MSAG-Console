#ifndef __SCAG_RULE_ENGINE_ACTION_PAIR__
#define __SCAG_RULE_ENGINE_ACTION_PAIR__

#include "scag/re/base/Action2.h"
#include "scag/re/base/StringField.h"

namespace scag2 {
namespace re {
namespace actions {

struct Pair {
    Pair() {}
    Pair(const std::string& n, const std::string& v):name(n), value(v) {}
    std::string name;
    std::string value;
};

class ActionPair : public Action {
public:
    ActionPair(bool readonlyPairName = false, bool readonlyPairValue = false):
               name_(*this, "name", true, readonlyPairName), value_(*this, "value", true, readonlyPairValue) {}
    ~ActionPair() {}
    void init(const SectionParams &params, PropertyObject propertyObject);
    bool run(ActionContext &context) { return true; };
    const char* opname() const { return "strings:pair"; }
    void setPair(const Pair& pair, ActionContext &context);
    Property* getNameProperty(ActionContext &context) { return name_.getProperty(context); }
    const char* getName(ActionContext &context);
    const char* getValue(ActionContext &context);
    Property* getValueProperty(ActionContext &context){ return value_.getProperty(context); }

protected:
    IParserHandler * StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory) { return NULL; }
    bool FinishXMLSubSection(const std::string &name) { return true; }

private:
    StringField name_;
    StringField value_;
};

}}}

#endif

