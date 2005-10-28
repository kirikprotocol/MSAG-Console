#include "CommandReader.h"
#include "core/network/Socket.hpp"
#include <memory.h>

namespace smsc {
namespace cluster {

using smsc::core::network::Socket;

void CommandReader::Start()
{
	stop = false;
	Thread::Start();
}

int CommandReader::Execute()
{
    smsc_log_info(logger, "Reader is strted");
	while(!stop)
	{
        //printf("reader, Execute, wait for command...\n");
        if (Socket *newSocket = sock->Accept())
        {
            smsc_log_info(logger, "Reader has accepted command");

            for( ;; ){

                try{
                    smsc_log_info(logger, "Reader reads command");
                    if(Command *cmd = readCommand(newSocket)){

                        smsc_log_info(logger, "Command %02X readed", cmd->getType());

                        if(cmd->getType() == GETROLE_CMD)
                            writeRole(newSocket, cmd);
                        else{

                            if(*role == SLAVE)
                                dispatcher->addCommand(cmd);
                            else
                                if(cmd)
                                    delete cmd;
                        }
                   
                    }
                }catch(Exception & e)
                {
                    smsc_log_info(logger, "Reader exception, %s", e.what());
                    break;
                }catch(...){
                    smsc_log_info(logger, "Reader exception, unexpected error");
                    break;
                }

            }
        }
        //printf("reader, Execute, command is accepted\n");
        
	}
    smsc_log_info(logger, "Reader is stoped");
	
	return 0;
}

void CommandReader::readHeader(Socket * socket, uint32_t &type, uint32_t &len)
{
    uint8_t buffer[8];

    try {
        read(socket, buffer, 8);

        uint32_t val;
        memcpy((void*)&val,     (const void*)buffer, 4);
        type = ntohl(val);

        memcpy((void*)&val,   (const void*)(buffer + 4), 4);
        len = ntohl(val);

    }catch(Exception& e){
        throw Exception("%s", e.what());
    }catch(...){
        throw Exception("Command read failed. Unexpected error.");
    }
}

Command* CommandReader::readCommand(Socket * socket)
{
    uint32_t type;
    uint32_t len;

    readHeader(socket, type, len);

    smsc_log_info(logger, "readCommand, Command header: type: %02X, len: %d", type, len);

    if(type == GETROLE_CMD){
        Command *cmd = new GetRoleCommand cmd( (int)( *role ));
        return cmd;
    }

    std::auto_ptr<uint8_t> buffer ( new uint8_t[len] );

    read(socket, (void*)buffer.get(), len);

    return Command::create((CommandType)type, (void*)buffer.get(), len);
}

void CommandReader::read(Socket * socket, void* buffer, int size)
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

    }catch(Exception e){
        throw Exception("%s", e.what());
    }catch(...){
        throw Exception("Command read failed. Unexpected error.");
    }
}

void CommandReader::writeRole(Socket* socket, Command *cmd)
{
    if(!cmd)
        return;

    uint32_t len = 0;
    std::auto_ptr<uint8_t> buff ( (uint8_t*)cmd->serialize(len) );
    int size = len + 8;
    std::auto_ptr<uint8_t> buffer( new uint8_t[size] );
    uint32_t type = htonl( cmd->getType() );
    memcpy((void*)buffer.get(), (const void*)&type, 4);
    uint32_t val32 = htonl(len);
    memcpy((void*)( buffer.get() + 4), (const void*)&val32, 4);
    memcpy((void*)( buffer.get() + 8), (const void*)buff.get(), len);
    delete cmd;

    try {
        if (!socket) throw Exception("Role send failed. Socket NULL!");
        int toWrite = size; const char* writeBuffer = (const char *)buffer.get();
        while (toWrite > 0) {
            int write = socket->canWrite(10);
            if (write == 0) throw Exception("Role send failed. Timeout expired.");
            else if (write > 0) {
                write = socket->Write(writeBuffer, toWrite);
                if (write > 0) { writeBuffer+=write; toWrite-=write; continue; }
            }
            throw Exception("Role send failed. Socket closed. %s", strerror(errno));
        }

    }catch(Exception e){
        throw Exception("%s", e.what());
    }catch(...){
        throw Exception("Command read failed. Unexpected error.");
    }
}

}
}
