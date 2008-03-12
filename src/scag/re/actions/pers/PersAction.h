#ifndef _PERS_ACTION_COMMIT_
#define _PERS_ACTION_COMMIT_

#include "scag/pers/Types.h"
#include <string>

#include "scag/re/actions/Action.h"
#include "scag/re/actions/LongCallAction.h"
#include "scag/pers/Property.h"

#include "scag/pers/PersClient.h"

namespace scag { namespace pers {

using namespace scag::re::actions;
using namespace scag::re;
using namespace scag::pers::client;

	class PersBatchAction : public LongCallAction
	{
	public:
		virtual bool batchPrepare(ActionContext& context, SerialBuffer& sb) = 0;
		virtual int batchResult(ActionContext& context, SerialBuffer& sb, bool transactMode = false) = 0;
	};
	
    class PersAction : public PersBatchAction
    {
    protected:
        PersCmd cmd;
        uint32_t lcm_cmd;
        
        ProfileType profile;
        std::string var;
        FieldType ftValue, ftModValue, ftOptionalKey;
        std::string value_str, mod_str;
        std::string result_str;
        TimePolicy policy;
        time_t final_date;
        time_t life_time;
        uint32_t mod;
		FieldType lifeTimeFieldType, finalDateFieldType, varType;
		std::string sLifeTime, sFinalDate;

        std::string status_str, msg_str;

        std::string optional_skey;
        uint32_t optional_ikey;
        std::string optional_key_str;
        bool has_optional_key;

        virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
        virtual bool FinishXMLSubSection(const std::string& name);

        const char* getStrCmd();
        ProfileType getProfileTypeFromStr(const std::string& str);
        TimePolicy getPolicyFromStr(const std::string& str);
        uint32_t getKey(const CommandProperty& cp, ProfileType pt);
		time_t parseFinalDate(const std::string& s);
		uint32_t parseLifeTime(const std::string& s);
        void getOptionalKey(const std::string& key_str);
        std::string getAbntAddress(const std::string& _address);
    public:
        PersAction() : cmd(PC_GET), final_date(-1), life_time(-1), policy(UNKNOWN), has_optional_key(false),
                       lifeTimeFieldType(ftUnknown), finalDateFieldType(ftUnknown), mod(0), optional_ikey(0) {}
        PersAction(PersCmd c) : cmd(c), final_date(-1), life_time(-1), policy(UNKNOWN), has_optional_key(false),
                                lifeTimeFieldType(ftUnknown), finalDateFieldType(ftUnknown), mod(0), optional_ikey(0) {}
        ~PersAction() {}
		virtual bool batchPrepare(ActionContext& context, SerialBuffer& sb);
		virtual int batchResult(ActionContext& context, SerialBuffer& sb, bool transactMode = false);
        virtual bool RunBeforePostpone(ActionContext& context);
        virtual void ContinueRunning(ActionContext& context);
        virtual void init(const SectionParams& params,PropertyObject propertyObject);
    };

}}

#endif
