#ifndef SCAG_RULE_ENGINE_HTTP_ADAPTER2
#define SCAG_RULE_ENGINE_HTTP_ADAPTER2

#include "scag/transport/http/base/HttpCommand2.h"
#include "ActionContext2.h"
#include "core/buffers/IntHash.hpp"

namespace scag2 {
namespace re {
namespace http {

using smsc::core::buffers::IntHash;
using smsc::core::buffers::Hash;
using bill::infrastruct::TariffRec;

class HttpCommandAdapter : public re::actions::CommandAccessor
{
public:

    static Hash<AccessType> RequestFieldsAccess;
    static Hash<AccessType> ResponseFieldsAccess;
    static Hash<AccessType> DeliveryFieldsAccess;
    static Hash<AccessType> InitRequestAccess();
    static Hash<AccessType> InitResponseAccess();
    static Hash<AccessType> InitDeliveryAccess();

    util::properties::Property* getRequestProperty(const std::string& name);
    util::properties::Property* getResponseProperty(const std::string& name);
    util::properties::Property* getDeliveryProperty(const std::string& name);

    HttpCommandAdapter(transport::http::HttpCommand& _command) :  command(_command)
    {
    }

    transport::http::HttpCommand& getCommand() { return command; };
    virtual transport::SCAGCommand& getSCAGCommand() { return command; }

    virtual void changed(util::properties::AdapterProperty& property);
    virtual util::properties::Property* getProperty(const std::string& name);
    virtual void delProperty( const std::string& name );

    virtual ~HttpCommandAdapter();

    static AccessType CheckAccess( int handlerType, const std::string& name );

private:
    transport::http::HttpCommand&            command;
    Hash<util::properties::AdapterProperty*> PropertyPool;

};

}}}

#endif // SCAG_RULE_ENGINE_HTTP_ADAPTER

