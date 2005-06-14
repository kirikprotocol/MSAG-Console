#ifndef SCAG_SESSIONS
#define SCAG_SESSIONS

#include <core/buffers/Hash.hpp>
#include <scag/util/properties/Properties.h>

namespace scag { namespace sessions 
{
    using core::buffers::Hash;

    using namespace scag::util::properties;

    class Session : public PropertyManager
    {
    private:

        Hash<NamedProperty>     properties;

        // TODO: Add session content & functionality

    public:

        virtual ~Session() {};
        
        // TODO: Implement PropertyManager interface
        virtual void changed(const NamedProperty& property);
        virtual NamedProperty* getProperty(const std::string& name);
    };
}}

#endif // SCAG_SESSIONS
