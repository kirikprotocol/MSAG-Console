#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>

#include "core/network/Socket.hpp"

/// POC for reading/writing from a socket

void usage( const char* p )
{
    std::printf("usage: %s (r|w) host port\n",p);
    std::terminate();
}


bool startReader( smsc::core::network::Socket* s );
bool startWriter( smsc::core::network::Socket* s );


void doReading( const char* host, int port )
{
    smsc::core::network::Socket sock;
    if ( 0 != sock.InitServer( host, port, 5) ) {
        std::printf("cannot initserver\n");
        return;
    }
    
    if ( 0 != sock.StartServer() ) {
        std::printf("cannot start server\n");
        return;
    }

    while (true) {
        smsc::core::network::Socket* s = sock.Accept();
        if (s) {
            char buf[80];
            s->GetPeer(buf);
            std::printf("incoming socket %s\n",buf);
            if (!startReader(s)) break;
        }
    }
    sock.Close();
}


void doWriting( const char* host, int port )
{
    for ( unsigned i = 0; i < 5; ++i ) {
        smsc::core::network::Socket* s = new smsc::core::network::Socket();
        if (0 != s->Init(host,port,10)) {
            std::printf("cannot init socket\n");
            return;
        }
        if (0 != s->Connect()) {
            std::printf("cannot connect\n");
            return;
        }
        startWriter(s);
    }
}


int main( int argc, char** argv )
{
    if ( argc < 4 ) {
        usage(argv[0]);
    }

    bool reading;
    if ( std::strcmp(argv[1],"r") == 0 ) {
        // reading
        reading = true;
    } else if ( std::strcmp(argv[1],"w") == 0 ) {
        // writing
        reading = false;
    } else {
        usage(argv[0]);
    }

    const char* host = argv[2];
    const unsigned short port = static_cast<unsigned short>(atoi(argv[3]));

    printf("%s %s:%u",reading ? "reading" : "writing", host, port);

    if ( reading ) {
        doReading( host, port );
    } else {
        doWriting( host, port );
    }
    return 0;
}


