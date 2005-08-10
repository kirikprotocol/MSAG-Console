#ifndef __SCAG_RULE_ENGINE_ACTION_IF__
#define __SCAG_RULE_ENGINE_ACTION_IF__

#include <list>

#include "ActionContext.h"
#include "Action.h"


namespace scag { namespace re { namespace actions 
{

class ActionIf : public Action
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
        std::string Operand1;
        std::string Operand2;
        IfOperations Operation;
        SingleParam() : Operation(opUnknown) {}
    };


    ActionIf(const ActionIf &);

    PropertyObject propertyObject;
    SingleParam singleparam;
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
    std::list<Action *> ThenActions;
    std::list<Action *> ElseActions;
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////
public:
    ActionIf();
    virtual void init(const SectionParams& params,PropertyObject _propertyObject);
    virtual ~ActionIf();
    virtual bool run(ActionContext& context);

};


}}}


#endif
