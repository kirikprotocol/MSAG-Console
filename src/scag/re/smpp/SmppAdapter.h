#ifndef SCAG_RULE_ENGINE_SMPP_ADAPTER
#define SCAG_RULE_ENGINE_SMPP_ADAPTER

//#include <SmppCommand.h> Where ???
#include <scag/util/properties/Properties.h>

namespace scag { namespace re { namespace smpp 
{
    using namespace scag::util::properties;

    class SmppCommandAdapter : public PropertyManager
    {
    private:

        SmppCommand&    command;

        // TODO: Add adapter content & functionality

    public:

        SmppCommandAdapter(SmppCommand& _command) 
            : PropertyManager(), command(_command) {};
        virtual ~SmppCommandAdapter() {};

        // TODO: Implement PropertyManager interface (Access to command fields)
        virtual void changed(const NamedProperty& property);
        virtual NamedProperty* getProperty(const std::string& name);
    };

}}}

#endif // SCAG_RULE_ENGINE_SMPP_ADAPTER
