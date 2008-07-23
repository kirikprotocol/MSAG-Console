#ifndef _SCAG_SESSIONS_SESSIONFINALIZER_H
#define _SCAG_SESSIONS_SESSIONFINALIZER_H

namespace scag {
namespace sessions2 {

    class Session;

    /// Interface of a session finalizer.
    /// An instance of finalizer is passed to a session store
    /// and finalize() method is invoked when session is expired.
    class SessionFinalizer
    {
    public:
        virtual ~SessionFinalizer() {}
        virtual void finalize( Session& ) = 0;
    };

} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONFINALIZER_H */
