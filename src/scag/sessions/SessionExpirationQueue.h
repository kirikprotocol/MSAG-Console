#ifndef _SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H
#define _SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H

namespace scag {
namespace sessions2 {

    class SessionKey;

    class SessionExpirationQueue 
    {
    public:
        virtual ~SessionExpirationQueue() {}
        virtual void scheduleExpire( time_t expirationTime,
                                     const SessionKey& key ) = 0;
    };


} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H */

