#ifndef SCAG_RULE_ENGINE_HTTP_ADAPTER2
#define SCAG_RULE_ENGINE_HTTP_ADAPTER2

#include "scag/transport/http/HttpCommand2.h"
#include "ActionContext2.h"
#include "core/buffers/IntHash.hpp"

namespace scag2 {
namespace re {
namespace http {

using namespace scag::util::properties;
using namespace transport::http;
using re::actions::CommandAccessor;
using smsc::core::buffers::IntHash;
using bill::infrastruct::TariffRec;

    class HttpCommandAdapter : public CommandAccessor
    {
        HttpCommand& command;
        Hash<AdapterProperty*> PropertyPool;

    public:

        static Hash<AccessType> RequestFieldsAccess;
        static Hash<AccessType> ResponseFieldsAccess;
        static Hash<AccessType> DeliveryFieldsAccess;
        static Hash<AccessType> InitRequestAccess();
        static Hash<AccessType> InitResponseAccess();
        static Hash<AccessType> InitDeliveryAccess();

        Property* getRequestProperty(const std::string& name);
        Property* getResponseProperty(const std::string& name);
        Property* getDeliveryProperty(const std::string& name);

        HttpCommandAdapter(HttpCommand& _command) :  command(_command)
        {
        }

        HttpCommand& getCommand() { return command; };
        virtual SCAGCommand& getSCAGCommand() { return command; }

        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);
        virtual ~HttpCommandAdapter();

        static AccessType CheckAccess(int handlerType, const std::string& name);
    };

}}}

#endif // SCAG_RULE_ENGINE_HTTP_ADAPTER

