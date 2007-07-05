#ifndef SCAG_RULE_ENGINE_SESSION_ADAPTER
#define SCAG_RULE_ENGINE_SESSION_ADAPTER

#include <scag/re/actions/ActionContext.h>

namespace scag { namespace re { namespace session
{
    using scag::re::actions::CommandAccessor;
    using scag::sessions::Session;    
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
