#include "PersServer.h"
#include "util/Exception.hpp"

namespace scag { namespace pers {

using smsc::util::Exception;
using smsc::logger::Logger;

#define MAX_PACKET_SIZE 100000

PersServer::PersServer(const char *persHost_, int persPort_, int maxClientCount_, CommandDispatcher *d)
    : log(Logger::getInstance("server")), persHost(""), persPort(0), isStopping(false), CmdDispatcher(d)
{
    persHost = persHost_;
    persPort = persPort_;
    maxClientCount = maxClientCount_;
    clientCount = 0;
}

PersServer::~PersServer()
{
    listener.remove(&sock);
    sock.Close();
    
    while(listener.count())
        remove_socket(listener.get(0));
            
    delete CmdDispatcher;
}

void PersServer::InitServer()
{
    if(sock.InitServer(persHost.c_str(), persPort, 10, 1, true))
        throw Exception("Failed to init socket server by host: %s, port: %d", persHost.c_str(), persPort);

    smsc_log_info(log, "Socket server is inited by host: %s, port: %d", persHost.c_str(), persPort);

    if(!listener.addR(&sock))
        throw Exception("Failed to init PersServer");
}

void PersServer::process_read_socket(Socket* s)
{
    int j;
    uint32_t k;
    SerialBuffer *sb = (SerialBuffer*)s->getData(0);

    if(sb->GetSize() < sizeof(uint32_t))
    {
        j = s->Read(tmp_buf, sizeof(uint32_t) - sb->GetSize());
        if(j > 0)
        {
            sb->Append(tmp_buf, j);
            if(sb->GetSize() >= sizeof(uint32_t))
            {
                j = sb->GetPos();
                sb->SetPos(0);
                k = sb->ReadInt32();
                if(k > MAX_PACKET_SIZE)
                {
                    smsc_log_warn(log, "Too big packet from client");
                    remove_socket(s);
                    return;
                }
                s->setData(1, (void*)k);
                sb->SetPos(j);
            }
        }
        else
        {
            if(errno != EWOULDBLOCK)
                remove_socket(s);
            return;
        }
    }
    if(sb->GetSize() >= sizeof(uint32_t))
    {
        k = (uint32_t)s->getData(1);
        j = k - sb->GetSize();
        j = s->Read(tmp_buf, j > 1024 ? 1024 : j);
        if(j > 0)
            sb->Append(tmp_buf, j);
        else  if(errno != EWOULDBLOCK)
        {
            remove_socket(s);
            return;
        }
        if(sb->GetSize() >= k)
        {
            CmdDispatcher->Execute(sb);
            sb->SetPos(0);
            listener.addW(s);
        }
    }
}

void PersServer::process_write_socket(Socket* s)
{
    int j;
    uint32_t i, len;
    SerialBuffer *sb = (SerialBuffer*)s->getData(0);

    len = sb->GetSize();

    j = s->Write(sb->GetCurPtr(), len - sb->GetPos());
    if(j > 0)
        sb->SetPos(sb->GetPos() + j);
    else if(errno != EWOULDBLOCK)
    {
        remove_socket(s);
        return;
    }
    if(sb->GetPos() >= len)
    {
        sb->Empty();
        listener.addR(s);
    }
}

void PersServer::remove_socket(Socket* s)
{
    char b[256];
    s->GetPeer(b);
    smsc_log_info(log, "Socket disconnected: %s", b);
    SerialBuffer *sb = (SerialBuffer*)s->getData(0);
    if(sb) delete sb;
    listener.remove(s);
    if(clientCount) clientCount--;
    s->Close();
    delete s;
}

int PersServer::Execute()
{
    Multiplexer::SockArray read, write, err;

    if(isStopped()) return 1;

    if( sock.StartServer())
        smsc_log_warn(log, "Server socket can't start");

    while(!isStopped())
    {
        if(listener.canReadWrite(read, write, err))
        {
            if(isStopped())
                break;

            for(int i = 0; i <= read.Count() - 1; i++)
            {
                if(read[i] == &sock)
                {
                    sockaddr_in addrin;
                    int sz = sizeof(addrin);
                    SOCKET s = accept(sock.getSocket(), (sockaddr*)&addrin, &sz);
                    if(s != -1)
                    {
                        Socket *sock1 = new Socket(s, addrin);

                        char b[256];
                        sock1->GetPeer(b);
                        smsc_log_info(log, "Client connected: %s", b);

                        if(clientCount >= maxClientCount)
                        {
                            sock1->Write("SB", 2);
                            delete sock1;
                            smsc_log_info(log, "Server busy sent. Disconnected.");
                        }
                        else
                        {
                            sock1->Write("OK", 2);
                            clientCount++;
                            sock1->setNonBlocking(1);
                            sock1->setData(0, new SerialBuffer());
                            listener.addR(sock1);
                        }
                    } else
                        smsc_log_error(log, "accept failed: %s", strerror(errno));
                } 
                else
                    process_read_socket(read[i]);
            }

            for(int i = 0; i <= write.Count() - 1; i++)
                process_write_socket(write[i]);

            for(int i = 0; i <= err.Count() - 1; i++)
                remove_socket(err[i]);
        }
    }

    return 1;
}

}}
