#ifndef _SCAG_PVSS_FLOODER_REQUESTGENERATOR_H
#define _SCAG_PVSS_FLOODER_REQUESTGENERATOR_H

#include <string>
#include <vector>
#include <list>
#include "core/buffers/IntHash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "scag/pvss/api/packets/AbstractCommand.h"
#include "scag/pvss/data/Property.h"
#include "scag/exc/IOException.h"

namespace scag2 {
namespace pvss {

class AbstractProfileRequest;

namespace flooder {

class RequestGenerator
{
public:
    RequestGenerator();

    ~RequestGenerator();

    /// randomize profile keys
    void randomizeProfileKeys( const std::string& addressFormat,
                               unsigned abonents );

    /// add a variable pattern
    void addPropertyPattern( unsigned idx, Property* prop ) /* throw (exceptions::IOException) */ ;

    /// parse command patterns
    void parseCommandPatterns( const std::string& patterns ) /* throw (exceptions::IOException) */ ;

    void clearCommandPatterns();

    /// get command patterns
    const std::vector< AbstractCommand* >& getPatterns() { return patterns_; }

    /// get the next profile key
    ProfileKey getProfileKey();

    /// NOTE: patternIndex is passed by ref to be able to wrap it when end-of-patterns reached.
    AbstractCommand* generateCommand( unsigned& patternIndex ); // , const ProfileKey* key = 0 );

    Property* getProperty( unsigned idx );
    
private:
    std::vector< AbstractCommand* >           patterns_; // owned
    smsc::core::buffers::IntHash< Property* > prophash_;
    std::list< Property* >                    proplist_;
    unsigned                                  abonents_;
    std::string                               addressFormat_;
    unsigned*                                 profileKeys_;  // owned
    smsc::core::synchronization::Mutex        keyMtx_;
    unsigned                                  abonentIdx_;
};

} // namespace flooder
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_FLOODER_REQUESTGENERATOR_H */
