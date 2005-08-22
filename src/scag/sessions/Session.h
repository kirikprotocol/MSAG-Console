#ifndef SCAG_SESSIONS_SESSION
#define SCAG_SESSIONS_SESSION

#include <inttypes.h>

#include <scag/bill/Bill.h>
#include <scag/util/properties/Properties.h>
#include "sms/sms.h"


namespace scag { namespace sessions 
{
    using scag::bill::Bill;

    using namespace scag::util::properties;
    using namespace smsc::sms;

    struct CSessionKey
    {
        int16_t UMR;
        smsc::sms::Address abonentAddr;
        bool operator == (const CSessionKey& sk) const 
        {
            return ((this->abonentAddr == sk.abonentAddr)&&(this->UMR == sk.UMR));
        }
    };    

    class Session : public PropertyManager
    {
        CSessionKey m_SessionKey;
        time_t   lastAccessTime;
        bool     bChanged;
        Hash<AdapterProperty *> PropertyHash;
    public:
        friend class SessionManagerImpl;

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

        CSessionKey getSessionKey() const {return m_SessionKey;}
        // TODO: Implement PropertyManager interface
        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);

    };
}}

#endif // SCAG_SESSIONS_SESSION
