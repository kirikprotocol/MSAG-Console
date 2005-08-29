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
        int16_t             USR; // User Session Reference
        smsc::sms::Address  abonentAddr;

        bool operator ==(const CSessionKey& sk) const 
        {
            return ((this->abonentAddr == sk.abonentAddr) && (this->USR == sk.USR));
        }
    };    

    class Session : public PropertyManager
    {
    protected:

        CSessionKey             m_SessionKey;
        time_t                  lastAccessTime;
        bool                    bChanged, bDestroy;
        EventMonitor            accessMonitor;
        int                     accessCount;

        Hash<AdapterProperty *> PropertyHash;
        
        virtual ~Session();
    
        friend class SessionGuard;
        bool Aquire();
        void Release();
        void Destroy();

    public:

        Session();
        
        CSessionKey getSessionKey() const {
            return m_SessionKey;
        }
        inline time_t getLastAccessTime() {
            return lastAccessTime;
        }
        inline bool isChanged() {
            return bChanged;
        }
        inline bool isInUse() {
            MutexGuard guard(accessMonitor);
            return (accessCount > 0);
        }
        
        void Expire();

        void attachBill(const Bill& bill);
        void detachBill(const Bill& bill);
        
        virtual void changed(AdapterProperty& property);
        virtual Property* getProperty(const std::string& name);
    };

    class SessionGuard
    {
        Session* session;
        bool     bAquire, bDestroy;

    public:

        SessionGuard(Session* _session) 
            : session(_session), bAquire(false), bDestroy(false) 
        {
            //bAquire = session->Aquire();
        }
        SessionGuard(const SessionGuard& sg)
        {
            // TODO: copy
            bAquire = session->Aquire();
        }
        ~SessionGuard() 
        {
            if (!session) return;
            if (bDestroy || !bAquire) session->Destroy();
            else session->Release();
        }
        
        Session* getSession() {
            return (bAquire ? session:0); 
        }
        inline void DestroySession() {
            bDestroy = true;
        }
    };
}}

#endif // SCAG_SESSIONS_SESSION

SessionGuard sg = store.GetSession(key);
Session* s = sg.getSession();
if (!s) {

}
