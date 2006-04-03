#ifndef SCAG_RULE_ENGINE_HTTP_ADAPTER
#define SCAG_RULE_ENGINE_HTTP_ADAPTER

#include <scag/transport/http/HttpCommand.h> 
#include <scag/util/properties/Properties.h>
#include <core/buffers/IntHash.hpp>

namespace scag { namespace re { namespace http {
    using namespace scag::util::properties;
    using namespace scag::transport::http;
    using smsc::core::buffers::IntHash;

    class HttpCommandAdapter : public PropertyManager
    {
        HttpCommand& command;
        Hash<AdapterProperty*> PropertyPool;

    public:

        static Hash<AccessType> RequestFieldsAccess;
        static Hash<AccessType> InitRequestAccess();

        Property* getRequestProperty(const std::string& name);
        Property* getResponseProperty(const std::string& name);

        HttpCommandAdapter(HttpCommand& _command) : PropertyManager(), command(_command)
        {
        }

        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);
        virtual ~HttpCommandAdapter();
        static AccessType CheckAccess(int handlerType, const std::string& name);
    };

}}}

#endif // SCAG_RULE_ENGINE_HTTP_ADAPTER

