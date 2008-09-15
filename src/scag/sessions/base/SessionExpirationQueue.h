#ifndef _SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H
#define _SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H

namespace scag2 {
namespace sessions {

    class Session;

    class SessionExpirationQueue 
    {
    public:
        virtual ~SessionExpirationQueue() {}
        virtual void scheduleExpire( time_t expirationTime,
                                     Session* session ) = 0;
    };


} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONEXPIRATIONQUEUE_H */

