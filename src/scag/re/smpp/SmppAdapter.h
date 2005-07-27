#ifndef SCAG_RULE_ENGINE_SMPP_ADAPTER
#define SCAG_RULE_ENGINE_SMPP_ADAPTER

#include <scag/transport/smpp/SmppCommand.h> 
#include <scag/util/properties/Properties.h>

namespace scag { namespace re { namespace smpp 
{
    using namespace scag::util::properties;
    using namespace scag::transport::smpp;


    class SmppCommandAdapter : public PropertyManager
    {
    private:

        SmppCommand&    command;
        Hash<NamedProperty *>  PropertyPul;

        NamedProperty * getPropertyFromPul(const std::string& name,int InitValue);
        NamedProperty * getPropertyFromPul(const std::string& name,const std::string& InitValue);


        NamedProperty * getProperty(const SMS& data,const std::string& name);
        NamedProperty * getProperty(const SubmitMultiSm& data,const std::string& name);
        NamedProperty * getProperty(const SubmitMultiResp& data,const std::string& name);
        NamedProperty * getProperty(const SmsResp& data,const std::string& name);
        NamedProperty * getProperty(const ReplaceSm& data,const std::string& name);
        NamedProperty * getProperty(const QuerySm& data,const std::string& name);
        NamedProperty * getProperty(const QuerySmResp& data,const std::string& name);
        NamedProperty * getProperty(const CancelSm& data,const std::string& name);
        NamedProperty * getProperty(const BindCommand& data,const std::string& name);
                                                                                        
    public:

        SmppCommandAdapter(SmppCommand& _command) 
            : PropertyManager(), command(_command) {};

        // TODO: Implement PropertyManager interface (Access to command fields)
        virtual void changed(const NamedProperty& property);
        virtual NamedProperty* getProperty(const std::string& name);
        virtual ~SmppCommandAdapter();
    };

}}}

#endif // SCAG_RULE_ENGINE_SMPP_ADAPTER
