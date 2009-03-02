#include <iostream>
#include <memory>

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

PingResponse* makePingResp( uint32_t s, Response::StatusType stat )
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

BatchCommand* makeBatch( uint32_t seq, bool trans )
{
    std::auto_ptr<BatchCommand> r( new BatchCommand(seq) );
    r->setTransactional(trans);
    return r.release();
}

DelCommand* makeDel( uint32_t seq, const std::string& var )
{
    std::auto_ptr<DelCommand> r(new DelCommand(seq));
    r->setVarName(var);
    return r.release();
}

GetCommand* makeGet( uint32_t seq, const std::string& var )
{
    std::auto_ptr<GetCommand> r(new GetCommand(seq));
    r->setVarName(var);
    return r.release();
}

SetCommand* makeSet( uint32_t seq, const std::string& var, const std::string& val )
{
    std::auto_ptr<SetCommand> r(new SetCommand(seq));
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
    AbstractProfileRequest* r = 
        new ProfileRequest<BatchCommand>
        ( push( push( push( makeBatch(3,true),
                            makeDel(4, "var1")),
                      makeGet(5,"var2")),
                makeSet(6,"var3", "хелло, ворлд")) );
    r->getProfileKey().setAbonentKey( ".0.1.79137654079" );
    packets.push_back(r);
}

int main()
{
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
