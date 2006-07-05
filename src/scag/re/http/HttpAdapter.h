#ifndef SCAG_RULE_ENGINE_HTTP_ADAPTER
#define SCAG_RULE_ENGINE_HTTP_ADAPTER

#include <scag/transport/http/HttpCommand.h> 
#include <scag/re/actions/ActionContext.h>
#include <core/buffers/IntHash.hpp>

namespace scag { namespace re { namespace http {
    using namespace scag::util::properties;
    using namespace scag::transport::http;
    using smsc::core::buffers::IntHash;
    using scag::re::actions::CommandAccessor;
    using scag::bill::infrastruct::TariffRec;

    class HttpCommandAdapter : public CommandAccessor
    {
        HttpCommand& command;
        Hash<AdapterProperty*> PropertyPool;

    public:

        static Hash<AccessType> RequestFieldsAccess;
        static Hash<AccessType> InitRequestAccess();

        Property* getRequestProperty(const std::string& name);
        Property* getResponseProperty(const std::string& name);

        HttpCommandAdapter(HttpCommand& _command) : CommandAccessor(), command(_command)
        {
        }

        HttpCommand& getCommand() { return command; };

        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);
        virtual ~HttpCommandAdapter();

        virtual void fillChargeOperation(smsc::inman::interaction::ChargeSms& op, TariffRec& tariffRec);
        virtual void fillRespOperation(smsc::inman::interaction::DeliverySmsResult& op, TariffRec& tariffRec);


        static AccessType CheckAccess(int handlerType, const std::string& name);
    };

}}}

#endif // SCAG_RULE_ENGINE_HTTP_ADAPTER

