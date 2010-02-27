#ifndef _SCAG_PVSS_FLOODER_REQUESTGENERATOR_H
#define _SCAG_PVSS_FLOODER_REQUESTGENERATOR_H

#include <string>
#include <vector>
#include <list>
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "scag/pvss/api/packets/ProfileCommand.h"
#include "scag/pvss/data/Property.h"
#include "scag/exc/IOException.h"
#include "scag/pvss/data/ProfileKey.h"

namespace scag2 {
namespace pvss {
namespace flooder {

class RequestGenerator
{
public:
    RequestGenerator();

    ~RequestGenerator();

    /// randomize profile keys
    void randomizeProfileKeys( const std::string& addressFormat,
                               unsigned abonents,
                               unsigned skip = unsigned(-1),
                               ScopeType stype = SCOPE_ABONENT,
                               unsigned offset = 0U );

    /// add a variable pattern
    void addPropertyPattern( unsigned idx, Property* prop ) /* throw (exceptions::IOException) */ ;

    /// parse command patterns
    void parseCommandPatterns( const std::string& patterns ) /* throw (exceptions::IOException) */ ;

    void clearCommandPatterns();

    /// get command patterns
    const std::vector< ProfileCommand* >& getPatterns() { return patterns_; }

    /// get the next profile key
    ProfileKey getProfileKey();

    /// NOTE: patternIndex is passed by ref to be able to wrap it when end-of-patterns reached.
    ProfileCommand* generateCommand( unsigned& patternIndex ); // , const ProfileKey* key = 0 );

    Property* getProperty( unsigned idx );
    
private:
    std::vector< ProfileCommand* >           patterns_; // owned
    smsc::core::buffers::IntHash< Property* > prophash_;
    std::list< Property* >                    proplist_;
    unsigned                                  abonents_;
    std::string                               addressFormat_;
    unsigned*                                 profileKeys_;  // owned
    smsc::core::synchronization::Mutex        keyMtx_;
    unsigned                                  abonentIdx_;
    ScopeType                                 scopeType_;
};

} // namespace flooder
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_FLOODER_REQUESTGENERATOR_H */
