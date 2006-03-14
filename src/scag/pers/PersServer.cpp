#include "PersServer.h"
#include "util/Exception.hpp"

namespace scag { namespace pers {

using smsc::util::Exception;
using smsc::logger::Logger;

PersServer::PersServer(const char *persHost_, int persPort_, int maxClientCount_, CommandDispatcher *d)
    : log(Logger::getInstance("perssrv")), persHost(""), persPort(0), isStopping(false), CmdDispatcher(d)
{
	InitServer(persHost_, persPort_, maxClientCount_);
}

PersServer::~PersServer()
{
	//todo delete sockets array
	delete CmdDispatcher;
}

void PersServer::InitServer(const char *persHost_, int persPort_, int maxClientCount_)
{
    persHost = persHost_;
    persPort = persPort_;
	maxClientCount = maxClientCount_;
	clientCount = 0;

    if(sock.InitServer(persHost.c_str(), persPort, 10))
        throw Exception("Failed to init socket server by host: %s, port: %d", persHost.c_str(), persPort);

    smsc_log_info(log, "Socket server is inited by host: %s, port: %d", persHost.c_str(), persPort);

    if(!listener.addR(&sock))
        throw Exception("Failed to init PersServer");
}

void PersServer::process_read_socket(Socket* s)
{
	uint32_t k, j;
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
				sb->Read((char*)&k, sizeof(uint32_t));
				s->setData(1, (void*)k);
				sb->SetPos(j);
			}
		}
		else if(j == -1)
			smsc_log_debug(log, "socket.Read failed: %s", strerror(errno));
	}
	else
	{
		k = (uint32_t)s->getData(1);
		j = k - sb->GetSize();
		j = s->Read(tmp_buf, j > 1024 ? 1024 : j);
		if(j > 0)
			sb->Append(tmp_buf, j);
		else if(j == -1)
		{
			smsc_log_debug(log, "socket.Read failed: %s", strerror(errno));
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
	uint32_t i, j, len;
	SerialBuffer *sb = (SerialBuffer*)s->getData(0);
	len = sb->GetSize();

	smsc_log_info(log, "Write Socket is ready");

	j = len - sb->GetPos();
	j = s->Write(sb->GetCurPtr(), j > 1024 ? 1024 : j);
	if(j > 0)
		sb->SetPos(sb->GetPos() + j);
	else if(j == -1 && errno != EWOULDBLOCK)
	{
		smsc_log_error(log, "socket.write failed: %s", strerror(errno));
		return;
	}
	if(sb->GetPos() >= len)
	{
		sb->Empty();
		listener.addR(s);
	}
}

int PersServer::Execute()
{
    Multiplexer::SockArray read, write, err;

    if(isStopping) return 1;

    if( sock.StartServer())
        smsc_log_warn(log, "Server socket can't start");

    while(!isStopped())
    {
        if(listener.canReadWrite(read, write, err))
		{
			if(isStopped())
				break;

            smsc_log_info(log, "There are ready sockets");

            for(int i = 0; i <= read.Count() - 1; i++)
			{
                if(read[i] == &sock)
				{
                    smsc_log_info(log, "Listen Socket is ready");
                    sockaddr_in addrin;
                    int sz = sizeof(addrin);
                    SOCKET s = accept(sock.getSocket(), (sockaddr*)&addrin, &sz);
                    if(s != -1)
					{
                        Socket *sock1 = new Socket(s, addrin);
						if(clientCount >= maxClientCount)
						{
							sock1->Write("SB", 2);
							delete sock1;
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
			{
				char b[256];
				err[i]->GetPeer(b);
				smsc_log_info(log, "Socket error: %s", b);
				SerialBuffer *sb = (SerialBuffer*)err[i]->getData(0);
				if(sb) delete sb;
				listener.remove(err[i]);
				delete err[i];
            }
        }
    }

    return 1;
}

}}
