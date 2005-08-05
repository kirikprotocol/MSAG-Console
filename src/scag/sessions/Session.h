#ifndef SCAG_SESSIONS_SESSION
#define SCAG_SESSIONS_SESSION

#include <inttypes.h>

#include <scag/bill/Bill.h>
#include <scag/util/properties/Properties.h>

namespace scag { namespace sessions 
{
    using scag::bill::Bill;

    using namespace scag::util::properties;

    class Session : public PropertyManager
    {
    private:
        
        uint16_t abSessionId;
        time_t   lastAccessTime;
        bool     bChanged;

    public:

        Session();
        virtual ~Session();
        
        inline time_t getLastAccessTime() {
            return lastAccessTime;
        }
        inline bool isChanged() {
            return bChanged;
        }

        // TODO: Implement billing transactions reg/unreg
        void attachBill(const Bill& bill);
        void detachBill(const Bill& bill);
        
        // TODO: Implement PropertyManager interface
        virtual void changed(const AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);

    };
}}

#endif // SCAG_SESSIONS_SESSION
