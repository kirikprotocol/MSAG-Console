#include <memory>
#include <iostream>
#include "scag/bill/ewallet/stream/StreamerImpl.h"
#include "scag/bill/ewallet/Open.h"
#include "scag/bill/ewallet/Client.h"
#include "scag/bill/ewallet/client/ClientCore.h"
#include "logger/Logger.h"

using namespace scag2::bill::ewallet;

int test1()
{
    std::auto_ptr< Streamer > p( new stream::StreamerImpl );

    Open packet;
    packet.setAgentId(102);
    packet.setUserId("bukind");
    packet.setWalletType("very-big-koshelek");
    packet.setDescription("full refund");
    packet.setAmount(-10000);
    packet.setTimeout(1000);

    Streamer::Buffer buffer;
    p->serialize(packet,buffer);
    
    std::cout << "packet = " << packet.toString() << std::endl;
    std::cout << "buffer = " << p->bufferDump(buffer) << std::endl;
    return 0;
}


class DummyHandler : public Client::ResponseHandler
{
public:
    virtual void handleResponse( std::auto_ptr<Request> request, std::auto_ptr<Response> resp ) 
    {
        std::cout << "response received: " << resp->toString() << std::endl;
    }
    virtual void handleError( std::auto_ptr<Request> request, const Exception& exc )
    {
        std::cout << "error: request " << request->toString() << " failed: " << exc.getMessage() << std::endl;
    }
};

int test2( Client::ResponseHandler& handler )
{
    std::auto_ptr< Client > client( new client::ClientCore( new proto::Config,
                                                            new stream::StreamerImpl ) );
    client->startup();

    std::auto_ptr<Request> pkt;
    {
        std::auto_ptr<Open> packet( new Open );
        packet->setAgentId(102);
        packet->setUserId("bukind");
        packet->setWalletType("very-big-koshelek");
        packet->setDescription("full refund");
        packet->setAmount(-10000);
        packet->setTimeout(1000);
        pkt.reset(packet.release());
    }
    sleep(1);
    client->processRequest( pkt, handler );
    return 0;
}


int main()
{
    smsc::logger::Logger::Init();
    DummyHandler handler;
    return test2(handler);
}
