#ifndef __SCAG_RULE_ENGINE_ACTION_IF2__
#define __SCAG_RULE_ENGINE_ACTION_IF2__

#include <list>

#include "scag/re/base/ActionContext2.h"
#include "scag/re/base/Action2.h"
#include "scag/re/base/LongCallAction2.h"

namespace scag2 {
namespace re {
namespace actions {

class ActionIf : public Action, ComplexActionLongCallHelper
{
    enum IfOperations
    {
        opUnknown,
        opEQ,
        opNE,
        opGT,
        opGE,
        opLT,
        opLE,

        opEQ_I,
        opGE_I,
        opLE_I,
        opNE_I,
        opGT_I,
        opLT_I
    };

    struct SingleParam
    {
        std::string strOperand1;
        std::string strOperand2;

        IfOperations Operation;
        SingleParam() : Operation(opUnknown) {}
    };


    ActionIf(const ActionIf &);

    PropertyObject propertyObject;
    SingleParam singleparam;

    FieldType ftSecondOperandFieldType;
    bool m_hasOP;

    bool CompareResultToBool(IfOperations op,int result);

    IfOperations GetOperationFromSTR(const std::string& str)
    {
        if (str == "eq") return opEQ;
        if (str == "ne") return opNE;
        if (str == "gt") return opGT;
        if (str == "ge") return opGE;
        if (str == "lt") return opLT;
        if (str == "le") return opLE;

        if (str == "eqi") return opEQ_I;
        if (str == "gei") return opGE_I;
        if (str == "lei") return opLE_I;
        if (str == "nei") return opNE_I;
        if (str == "gti") return opGT_I;
        if (str == "lti") return opLT_I;

        return opUnknown;
    };


    bool FillThenSection;
    bool FillElseSection;
    std::vector<Action *> ThenActions;
    std::vector<Action *> ElseActions;
protected:

//////////////IParserHandler Interfase///////////////////////

    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////

    //virtual bool RunActionVector(ActionContext& context, LongCallContext& longCallContext, std::vector<Action *>& actions, Logger * logger);

public:

    ActionIf();
    virtual void init(const SectionParams& params,PropertyObject _propertyObject);
    virtual ~ActionIf();
    virtual bool run(ActionContext& context);
};

}}}

#endif
