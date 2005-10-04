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
    sock.StartServer();

    while(!stop)
    {

        if (Socket * newSocket = sock.Accept())
        {

            smsc_log_info(logger, "Command is accepted" );

            try{

                smsc_log_info(logger, "Read a command..." );
                int res = readCommand(newSocket);
                smsc_log_info(logger, "Command is read, res: %d", res );

                if(res == 0){

                    Interconnect * icon = Interconnect::getInstance();
                    if(icon)
                        icon->changeRole(MASTER);

                }else if(res == 1){

                    stopSmsc = true;
                    stop = true;

                }
                   
                
            }catch(...){
                break;
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
    sock.InitServer(host.c_str(), port, 10);
};

void AgentListener::Stop()
{
    stop = true;
    WaitFor();
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
            int read = socket->canRead(10);
            if (read == 0) throw Exception("Command read failed. Timeout expired.");
                else if (read > 0) {
                    read = socket->Read(readBuffer, toRead);
                    smsc_log_info(logger, "read, read: %d, toRead: %d", read, toRead );
                    if (read > 0) { readBuffer+=read; toRead-=read; continue; }
                }
                throw Exception("Command read failed. Socket closed. %s", strerror(errno));
        }

    }catch(Exception e){
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
