#ifndef _PERS_ACTION_COMMIT_
#define _PERS_ACTION_COMMIT_

#include "Types.h"
#include <string>

#include "scag/re/actions/Action.h"
#include "Property.h"

namespace scag { namespace pers {

using namespace scag::re::actions;
using namespace scag::re;
using smsc::logger::Logger;

    class PersAction : public Action
    {
    protected:
        PersCmd cmd;
        ProfileType profile;
        std::string var;
        FieldType ftValue;
        std::wstring value;
        std::string value_str;
        TimePolicy policy;
        time_t final_date;
        time_t life_time;

        static Logger * logger;

        virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
        virtual bool FinishXMLSubSection(const std::string& name);

        const char* getStrCmd();
        ProfileType getProfileTypeFromStr(std::string& str);
        TimePolicy getPolicyFromStr(std::string& str);
        uint32_t getKey(const CommandProperty& cp, ProfileType pt);
    public:
        PersAction() : cmd(PC_GET) {}
        PersAction(PersCmd c) : cmd(c) {}
        virtual bool run(ActionContext& context);
        virtual void init(const SectionParams& params,PropertyObject propertyObject);
    };

}}


#endif


