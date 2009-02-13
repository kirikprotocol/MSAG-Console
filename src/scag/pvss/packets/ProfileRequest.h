#ifndef _SCAG_PVSS_BASE_PROFILEREQUEST_H
#define _SCAG_PVSS_BASE_PROFILEREQUEST_H

#include "AbstractProfileRequest.h"
#include "Command.h"

namespace scag2 {
namespace pvss {

///
/// Abstract profile request class
///
template < class T > class ProfileRequest : public AbstractProfileRequest
{
public:
    ProfileRequest( T* cmd ) : AbstractProfileRequest(), command_(cmd) {}
    ProfileRequest( T* cmd, const ProfileKey& profileKey ) : AbstractProfileRequest(profileKey), command_(cmd) {}
    
    virtual const T* getCommand() const { return command_; }
    virtual T* getCommand() { return command_; }

private:
    T* command_;
};

template < class T > ProfileRequest< T >* createProfileRequest( T* cmd )
{
    return new ProfileRequest< T >( cmd );
}

} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_BASE_PROFILEREQUEST_H */
