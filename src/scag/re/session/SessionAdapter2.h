#ifndef SCAG_RULE_ENGINE_SESSION_ADAPTER2
#define SCAG_RULE_ENGINE_SESSION_ADAPTER2

#include "scag/re/actions/ActionContext2.h"

namespace scag2 {
namespace re {
namespace session {

    using actions::CommandAccessor;
    using sessions::Session;
    using namespace scag::util::properties;

    class SessionAdapter : public CommandAccessor
    {
		Session& session;
		AdapterProperty* statusProp;
    public:

        SessionAdapter(Session& se) : session(se), statusProp(NULL)
		{
		}

        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);
	
        virtual SCAGCommand& getSCAGCommand() { throw SCAGException("Cannot use command in the session adapter"); }        
	
        virtual ~SessionAdapter();
        static AccessType CheckAccess(int handlerType,const std::string& name);
    };

}}}

#endif
