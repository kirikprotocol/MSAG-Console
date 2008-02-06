#ifndef _PERS_ACTION_COMMIT_
#define _PERS_ACTION_COMMIT_

#include "scag/pers/Types.h"
#include <string>

#include "scag/re/actions/Action.h"
#include "scag/re/actions/LongCallAction.h"
#include "scag/pers/Property.h"

namespace scag { namespace pers {

using namespace scag::re::actions;
using namespace scag::re;

	class PersBatchAction : public LongCallAction
	{
	public:
		virtual bool batchPrepare(ActionContext& context, SerialBuffer& sb) = 0;
		virtual void batchResult(ActionContext& context, SerialBuffer& sb) = 0;
	};
	
    class PersAction : public PersBatchAction
    {
    protected:
        PersCmd cmd;
        uint32_t lcm_cmd;
        
        ProfileType profile;
        std::string var;
        FieldType ftValue, ftModValue;
        std::string value_str, mod_str;
        std::string result_str;
        TimePolicy policy;
        time_t final_date;
        time_t life_time;
        uint32_t mod;
		FieldType lifeTimeFieldType, finalDateFieldType, varType;
		std::string sLifeTime, sFinalDate;

        std::string status_str, msg_str;

        virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
        virtual bool FinishXMLSubSection(const std::string& name);

        const char* getStrCmd();
        ProfileType getProfileTypeFromStr(const std::string& str);
        TimePolicy getPolicyFromStr(const std::string& str);
        uint32_t getKey(const CommandProperty& cp, ProfileType pt);
		time_t parseFinalDate(const std::string& s);
		uint32_t parseLifeTime(const std::string& s);
    public:
        PersAction() : cmd(PC_GET), final_date(-1), life_time(-1), policy(UNKNOWN), lifeTimeFieldType(ftUnknown), finalDateFieldType(ftUnknown) {}
        PersAction(PersCmd c) : cmd(c), final_date(-1), life_time(-1), policy(UNKNOWN), lifeTimeFieldType(ftUnknown), finalDateFieldType(ftUnknown) {}
        ~PersAction() {}
		virtual bool batchPrepare(ActionContext& context, SerialBuffer& sb);
		virtual void batchResult(ActionContext& context, SerialBuffer& sb);
        virtual bool RunBeforePostpone(ActionContext& context);
        virtual void ContinueRunning(ActionContext& context);
        virtual void init(const SectionParams& params,PropertyObject propertyObject);
    };

}}

#endif
