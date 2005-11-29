#include "cluster/AgentListener.h"
#include "cluster/Interconnect.h"
#include <signal.h>

namespace smsc {
namespace system {

extern "C" void sigShutdownHandler(int signo);

}
}

namespace smsc{
namespace cluster {

using smsc::core::network::Socket;
using smsc::system::sigShutdownHandler;

void AgentListener::Start()
{
    stop = false;
    Thread::Start();
}

int AgentListener::Execute()
{
    smsc_log_info(logger, "Start server..." );
    if( sock.StartServer() ){
        smsc_log_info(logger, "Can't start socket server" );
        return 0;
    }
    smsc_log_info(logger, "Server started" );

    while(!stop)
    {

        smsc_log_info(logger, "Wait for a command...." );
        std::auto_ptr<Socket> newSocket(sock.Accept());
        if (newSocket.get())
        {

            smsc_log_info(logger, "Command is accepted" );

            for( ;; ){

                try{

                    smsc_log_info(logger, "Read a command..." );
                    int res = -1;
                    res = readCommand(newSocket.get());
                    smsc_log_info(logger, "Command is read, res: %d", res );

                    if(res == 0){

                        Interconnect * icon = Interconnect::getInstance();
                        if(icon)
                            icon->changeRole(MASTER);

                    }else if(res == 1){

                        stopSmsc = true;
                        stop = true;
                        sock.Close();
                        break;

                    }


                }catch(...){
                    smsc_log_info(logger, "Exception during read command." );
                    smsc_log_info(logger, "Restarts socket server ..." );
                    if( sock.StartServer() )
                        smsc_log_info(logger, "Can't start socket server" );
                    else
                        smsc_log_info(logger, "Socket server started" );

                    break;
                }

            }


        }
    }

    if(stopSmsc)
        kill(pid, SIGTERM);
    return 0;
}

void AgentListener::init(std::string host, int port, pid_t pid_)
{
    pid = pid_;
    sock.InitServer(host.c_str(), port, 0);
};

void AgentListener::Stop()
{
    smsc_log_info(logger, "agent listener stopping" );
    sock.Close();
    stop = true;
    WaitFor();
    smsc_log_info(logger, "agent listener stopped" );
}

int AgentListener::readCommand(Socket * socket)
{
    uint8_t buffer[6];
    smsc_log_info(logger, "readCommand, read..." );
    read(socket, (void*)&buffer, 6);
    smsc_log_info(logger, "readCommand, command is read" );

    smsc_log_info(logger, "b[0]: %d, b[1]: %d, b[2]: %d, b[3]: %d", buffer[0], buffer[1], buffer[2], buffer[3] );

    // Checks signature
    if(buffer[0] != 17 || buffer[1] != 32 || buffer[2] != 7 || buffer[3] != 152)
        return -1;

    uint16_t val;

    memcpy((void*)&val, (const void*)(buffer + 4), 2);

    val = ntohs(val);

    smsc_log_info(logger, "val: %d", val );

    return val;

}

void AgentListener::read(Socket * socket, void* buffer, int size)
{
    try {
        if(!socket) throw Exception("Command read failed. Socket pointer is NULL.");
        int toRead = size; char* readBuffer = (char *)buffer;
        while (toRead > 0) {
            int read = socket->canRead(60);
            if (read == 0) continue;
                else if (read > 0) {
                    read = socket->Read(readBuffer, toRead);
                    if (read > 0) { readBuffer+=read; toRead-=read; continue; }
                }
                throw Exception("Command read failed. Socket closed. %s", strerror(errno));
        }

    }catch(Exception& e){
        throw Exception("%s", e.what());
    }catch(...){
        throw Exception("Command read failed. Unexpected error.");
    }
}

void AgentListener::shutdownSmsc()
{
        sigShutdownHandler(SIGTERM);
}

}
}
