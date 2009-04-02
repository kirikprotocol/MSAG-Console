#include <iostream>
#include <memory>

#include "logger/Logger.h"
#include "PingRequest.h"
#include "PingResponse.h"
#include "AuthRequest.h"
#include "ProfileRequest.h"
#include "DelCommand.h"
#include "GetCommand.h"
#include "SetCommand.h"
#include "BatchCommand.h"

using namespace scag2::pvss;

PingRequest* makePing( uint32_t s )
{
    return new PingRequest(s);
}

PingResponse* makePingResp( uint32_t s, uint8_t stat )
{
    std::auto_ptr<PingResponse> r( new PingResponse(s) );
    r->setStatus(stat);
    return r.release();
}

AuthRequest* makeAuth( uint32_t seq, uint8_t vers,
                       const std::string& login,
                       const std::string& password,
                       const std::string& name )
{
    std::auto_ptr<AuthRequest> r( new AuthRequest(seq) );
    r->setProtocolVersion(vers);
    r->setLogin( login );
    r->setPassword( password );
    r->setName( name );
    return r.release();
}

BatchCommand* makeBatch( bool trans )
{
    std::auto_ptr<BatchCommand> r( new BatchCommand );
    r->setTransactional(trans);
    return r.release();
}

DelCommand* makeDel( const std::string& var )
{
    std::auto_ptr<DelCommand> r(new DelCommand);
    r->setVarName(var);
    return r.release();
}

GetCommand* makeGet( const std::string& var )
{
    std::auto_ptr<GetCommand> r(new GetCommand);
    r->setVarName(var);
    return r.release();
}

SetCommand* makeSet( const std::string& var, const std::string& val )
{
    std::auto_ptr<SetCommand> r(new SetCommand);
    r->setVarName(var);
    r->setStringValue(val);
    return r.release();
}

BatchCommand* push( BatchCommand* batch, BatchRequestComponent* cmd )
{
    batch->addComponent( cmd );
    return batch;
}

void makePackets( std::vector< Packet* >& packets )
{
    packets.push_back( makePing(1) );
    packets.push_back( makePingResp(1,Response::BAD_REQUEST) );
    packets.push_back( makeAuth(2,2,"login", "password", "msag") );
    ProfileRequest* r = new ProfileRequest(10);
    r->setCommand( push( push( push(makeBatch(true),
                                    makeDel("var1")),
                               makeGet("var2")),
                         makeSet("var3", "хелло, ворлд")) );
    r->getProfileKey().setAbonentKey( ".1.1.79137654079" );
    packets.push_back(r);
}

int main()
{
    smsc::logger::Logger::Init();

    std::vector< Packet* > packets;
    makePackets( packets );
    for ( std::vector< Packet* >::iterator i = packets.begin();
          i != packets.end();
          ++i ) {
        std::cout << "cmd#" << (i-packets.begin());
        std::cout << ": " << (*i)->toString();
        std::cout << " isValid=" << (*i)->isValid();
        std::cout << std::endl;
        delete *i;
        *i = 0;
    }
    return 0;
}
