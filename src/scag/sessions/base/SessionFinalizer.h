#ifndef _SCAG_SESSIONS_SESSIONFINALIZER_H
#define _SCAG_SESSIONS_SESSIONFINALIZER_H

namespace scag2 {
namespace sessions {

class Session;

/// Interface of a session finalizer.
/// An instance of finalizer is passed to a session store
/// and finalize() method is invoked when session is expired.
class SessionFinalizer
{
public:
    virtual ~SessionFinalizer() {}

    /// tries to finalize the session.
    /// return true if the session is finalized and should be disposed.
    /// otherwise, return false indicating that session is in the process of
    /// being finalized.
    /// The end of finalization in this case will be reported via
    /// SessionStore::sessionFinalized().
    virtual bool finalize( Session& s ) = 0;
};

} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONFINALIZER_H */
