#ifndef _SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H
#define _SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H

namespace scag2 {
namespace sessions {

    class SessionKey;

    class SessionExpirationQueue 
    {
    public:
        virtual ~SessionExpirationQueue() {}
        virtual void scheduleExpire( time_t expirationTime,
                                     time_t lastaccessTime,
                                     const SessionKey& key ) = 0;
    };


} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H */

