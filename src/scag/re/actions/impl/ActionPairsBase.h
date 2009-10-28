#ifndef __SCAG_RULE_ENGINE_ACTION_PAIRSBASE__
#define __SCAG_RULE_ENGINE_ACTION_PAIRSBASE__

#include "scag/re/base/Action2.h"
#include "scag/re/base/StringField.h"
#include "ActionPair.h"

#include <vector>

namespace scag2 {
namespace re {
namespace actions {

class ActionPairsBase : public Action {
public:
    ActionPairsBase(bool readonlyPairName = false, bool readonlyPairValue = false):nameDel_(*this, "name_delimiter", true, true),
                      pairDel_(*this, "pair_delimiter", true, true), readonlyPairName_(readonlyPairName), readonlyPairValue_(readonlyPairValue) {}
    virtual ~ActionPairsBase();
    virtual void init(const SectionParams &params, PropertyObject propertyObject);

protected:
    IParserHandler * StartXMLSubSection(const std::string &name, const SectionParams &params, const ActionFactory &factory);
    bool FinishXMLSubSection(const std::string &name);
    Pair splitPair(const std::string& pair, const std::string& nameDel);

protected:
    StringField nameDel_;
    StringField pairDel_;
    PropertyObject pobj_;
    std::vector<ActionPair*> pairActions_;
    bool readonlyPairName_;
    bool readonlyPairValue_;
};

}}}

#endif

