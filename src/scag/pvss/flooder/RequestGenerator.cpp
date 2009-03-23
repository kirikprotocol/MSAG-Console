#include <memory>
#include <cstdlib>
#include <climits>
#include <algorithm>
#include "RequestGenerator.h"
#include "scag/util/Drndm.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncModCommand.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/api/packets/AbstractProfileRequest.h"

using namespace smsc::core::synchronization;

namespace {

using namespace scag2::util;

class Myrand 
{
public:
    uint64_t operator () ( uint64_t max ) {
        return Drndm::uniform( max, Drndm::getRnd().getNextNumber() );
    }
};

Myrand myrand;

}

namespace scag2 {
namespace pvss {
namespace flooder {

using namespace exceptions;

RequestGenerator::RequestGenerator() :
abonents_(1),
addressFormat_(".1.1.791%08u"),
profileKeys_(0),
abonentIdx_(0)
{}


RequestGenerator::~RequestGenerator() {
    clearCommandPatterns();
    for ( std::list<Property*>::const_iterator i = proplist_.begin();
          i != proplist_.end();
          ++i ) {
        delete *i;
    }
    delete [] profileKeys_;
}


void RequestGenerator::randomizeProfileKeys( const std::string& addressFormat,
                                             unsigned abonents )
{
    abonents_ = abonents;
    addressFormat_ = addressFormat;

    // randomizing the seed
    Drndm::getRnd().setSeed(uint64_t(time(0)));

    delete profileKeys_;
    profileKeys_ = 0;
    if ( abonents > 1000000 ) {
        // too many abonents, using randomization on-the-fly
        return;
    }
    profileKeys_ = new unsigned[abonents_];
    for ( unsigned i = 0; i < abonents; ++i ) {
        profileKeys_[i] = i;
    }
    std::random_shuffle( profileKeys_, profileKeys_+abonents, ::myrand );
}


void RequestGenerator::addPropertyPattern( unsigned idx, Property* prop ) throw (exceptions::IOException)
{
    std::auto_ptr< Property > p( prop );
    if ( prophash_.Exist(idx) ) throw exceptions::IOException("index %d is already occupied", idx);
    prophash_.Insert(idx,prop);
    proplist_.push_back(p.release());
}


void RequestGenerator::parseCommandPatterns( const std::string& patterns ) throw (exceptions::IOException)
{
    std::auto_ptr<BatchRequestComponent> cmd;
    std::auto_ptr<BatchCommand> batchCmd;
    unsigned batchCount = 0;
    unsigned modulus = 0;
    for ( size_t pos = 0; pos+1 < patterns.size(); ) {
        char cmdtype = patterns[pos++];
        char* endptr;
        unsigned long int cmdnumber = strtoul(patterns.c_str()+pos, &endptr, 10);
        if ( cmdnumber == ULONG_MAX ) 
            throw IOException("reading cmdnumber overflow");
        pos = endptr - patterns.c_str();
        if ( cmdtype == 'b' || cmdtype == 'B' ) {
            // batch
            if ( batchCount != 0 ) throw IOException("batch command appeared while previous batch is active");
            batchCmd.reset( new BatchCommand );
            batchCount = unsigned(cmdnumber);
            if ( batchCount <= 0 ) throw IOException("batch count cannot be less than or equal to 0");
            batchCmd->setTransactional( cmdtype == 'b' );
        } else if ( cmdtype == 'm' ) {
            modulus = unsigned(cmdnumber);
        } else {
            // non-batch
            const Property* prop = getProperty(unsigned(cmdnumber));
            if ( ! prop ) throw IOException("property #%d is not found", unsigned(cmdnumber));

            switch ( cmdtype ) {
            case 'g' : { // get
                GetCommand* kmd = new GetCommand;
                cmd.reset(kmd);
                kmd->setVarName(prop->getName());
                break;
            }
            case 's' : { // set
                SetCommand* kmd = new SetCommand;
                cmd.reset(kmd);
                kmd->setProperty(new Property(*prop));
                break;
            }
            case 'd' : { // del
                DelCommand* kmd = new DelCommand;
                cmd.reset(kmd);
                kmd->setVarName(prop->getName());
                break;
            }
            case 'i' : { // inc
                IncCommand* kmd = new IncCommand;
                cmd.reset( kmd );
                if ( prop->getType() != INT )
                    throw IOException("not an integer type for inc command");
                kmd->setProperty(new Property(*prop));
                break;
            }
            case 'I' : { // incmod
                IncModCommand* kmd = new IncModCommand;
                cmd.reset(kmd);
                if ( prop->getType() != INT )
                    throw IOException("not an integer type for incmod command");
                if ( modulus <= 0 ) throw IOException("modulus is not set for incmod");
                kmd->setProperty(new Property(*prop));
                kmd->setModulus(modulus);
                break;
            }
            } // switch
        }

        if ( ! cmd.get() ) continue;
        if ( batchCmd.get() ) {
            // batch command
            batchCmd->addComponent( cmd.release() );
            if ( ! --batchCount ) {
                patterns_.push_back( batchCmd.release() );
            }
        } else {
            // single command
            patterns_.push_back( cmd.release() );
        }
    }

    // post processing check
    if ( batchCmd.get() )
        throw IOException( "not all components for batch command specified");
}


void RequestGenerator::clearCommandPatterns()
{
    for ( std::vector< AbstractCommand* >::const_iterator i = patterns_.begin();
          i != patterns_.end(); ++i ) {
        delete *i;
    }
    patterns_.clear();
}


ProfileKey RequestGenerator::getProfileKey()
{
    unsigned key;
    {
        MutexGuard mg(keyMtx_);
        if ( profileKeys_ ) {
            if ( abonentIdx_ >= abonents_ ) abonentIdx_ = 0;
            key = profileKeys_[abonentIdx_++];
        } else {
            key = unsigned(::myrand(abonents_));
        }
    }
    char buf[40];
    snprintf( buf, sizeof(buf), addressFormat_.c_str(), key );
    ProfileKey pk;
    pk.setAbonentKey( buf );
    return pk;
}


Property* RequestGenerator::getProperty( unsigned idx )
{
    Property** ptr = prophash_.GetPtr(idx);
    if ( ! ptr ) return 0;
    return *ptr;
}


AbstractProfileRequest* RequestGenerator::generate( unsigned& patternIndex, const ProfileKey* key )
{
    assert( patterns_.size() > 0 );
    AbstractProfileRequest* res = 0;
    if ( patternIndex < patterns_.size() ) {
        res = AbstractProfileRequest::create(patterns_[patternIndex++]->clone());
        if ( key && res ) res->setProfileKey(*key);
    } else {
        patternIndex = 0;
    }
    return res;
}


} // namespace flooder
} // namespace pvss
} // namespace scag2
