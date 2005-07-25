#ifndef __SCAG_RULE_ENGINE_ACTION_IF__
#define __SCAG_RULE_ENGINE_ACTION_IF__

#include "ActionContext.h"
#include "Action.h"

#include <scag/re/actions/IParserHandler.h>
#include <core/buffers/IntHash.hpp>

using smsc::core::buffers::IntHash;
using namespace std;


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

        if (str == "==") return opEQ_I;
        if (str == ">=") return opGE_I;
        if (str == "<=") return opLE_I;
        if (str == "!=") return opNE_I;
        if (str == ">") return opGT_I;
        if (str == "<") return opLT_I;

        return opUnknown;
    };


    bool FillThenSection;
    bool FillElseSection;
    IntHash<Action *> ThenActions;
    IntHash<Action *> ElseActions;
protected:
//////////////IParserHandler Interfase///////////////////////
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
//////////////IParserHandler Interfase///////////////////////
public:
    ActionIf();
    virtual void init(const SectionParams& params);
    virtual ~ActionIf();
    virtual bool run(ActionContext& context);

};


}}}


#endif
