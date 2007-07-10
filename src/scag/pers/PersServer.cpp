#include "PersServer.h"
#include "util/Exception.hpp"

namespace scag { namespace pers {

using smsc::util::Exception;

class ConnectionContext{
public:
	ConnectionContext() : wantRead(true), lastActivity(time(NULL)) {};
	SerialBuffer inbuf, outbuf;
	bool wantRead;
	time_t lastActivity;
	uint32_t packetLen;
};
 
#define MAX_PACKET_SIZE 100000

PersServer::PersServer(const char *persHost_, int persPort_, int maxClientCount_, int timeout_, CommandDispatcher *d)
    : log(Logger::getInstance("server")), persHost(""), persPort(0), isStopping(false), CmdDispatcher(d)
{
    persHost = persHost_;
    persPort = persPort_;
    maxClientCount = maxClientCount_;
    clientCount = 0;
	timeout = timeout_;
}

PersServer::~PersServer()
{
    listener.remove(&sock);
    sock.Close();
    
    while(listener.count())
        removeSocket(listener.get(0));
            
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

void PersServer::processReadSocket(Socket* s)
{
    int j;
    long k;
	ConnectionContext* ctx = (ConnectionContext*)s->getData(0);
    SerialBuffer& sb = ctx->inbuf;

    if(sb.GetSize() < sizeof(uint32_t))
    {
        j = s->Read(tmp_buf, sizeof(uint32_t) - sb.GetSize());
        smsc_log_debug(log, "read(len) %u bytes from %p", j, s);
        if(j > 0)
        {
            sb.Append(tmp_buf, j);
            if(sb.GetSize() >= sizeof(uint32_t))
            {
                j = sb.GetPos();
                sb.SetPos(0);
                ctx->packetLen = sb.ReadInt32();
                smsc_log_debug(log, "%d bytes will be read from %x", ctx->packetLen, s);
                if(ctx->packetLen > MAX_PACKET_SIZE)
                {
                    smsc_log_warn(log, "Too big packet from client");
                    removeSocket(s);
                    return;
                }
                sb.SetPos(j);
            }
        }
        else
        {
            if(j) smsc_log_debug(log, "Error: %s(%d)", strerror(errno), errno);
//            if(errno != EWOULDBLOCK)
                removeSocket(s);
            return;
        }
    }
    if(sb.GetSize() >= sizeof(uint32_t))
    {
        j = ctx->packetLen - sb.GetSize();
        j = s->Read(tmp_buf, j > 1024 ? 1024 : j);
        smsc_log_debug(log, "read %u bytes from %p", j, s);
        if(j > 0)
            sb.Append(tmp_buf, j);
        else if(errno != EWOULDBLOCK)
        {
            if(j) smsc_log_debug(log, "Error: %s(%d)", strerror(errno), errno);
            removeSocket(s);
            return;
        }
        if(sb.GetSize() >= ctx->packetLen)
        {
            smsc_log_debug(log, "read from socket: len=%d, data=%s", sb.length(), sb.toString().c_str());
			ctx->outbuf.Empty();
            ctx->inbuf.SetPos(4);
            CmdDispatcher->Execute(ctx->inbuf, ctx->outbuf);
			ctx->lastActivity = time(NULL);			
            ctx->outbuf.SetPos(0);
            listener.addRW(s);
			ctx->wantRead = false; // indicate that we want write and read is only for EOF signalling
        }
    }
}

void PersServer::processWriteSocket(Socket* s)
{
    int j;
    uint32_t i, len;
	ConnectionContext* ctx = (ConnectionContext*)s->getData(0);
    SerialBuffer& sb = ctx->outbuf;

    len = sb.GetSize();

    smsc_log_debug(log, "write %u bytes to %p, GetCurPtr: %x, GetPos: %d", len, s, sb.GetCurPtr(), sb.GetPos());
    j = s->Write(sb.GetCurPtr(), len - sb.GetPos());
    if(j > 0)
        sb.SetPos(sb.GetPos() + j);
    else //if(errno != EWOULDBLOCK)
    {
        smsc_log_debug(log, "Error: %s(%d)", strerror(errno), errno);
        removeSocket(s);
        return;
    }
    if(sb.GetPos() >= len)
    {
        smsc_log_debug(log, "written to socket: len=%d, data=%s", sb.length(), sb.toString().c_str());
        ctx->inbuf.Empty();
		ctx->lastActivity = time(NULL);
        listener.addR(s);
		ctx->wantRead = true;
    }
}

void PersServer::removeSocket(Socket* s, int i)
{
    char b[256];
    s->GetPeer(b);
    smsc_log_info(log, "Socket disconnected: %s. Client count: %u", b, clientCount - 1);
    ConnectionContext *ctx = (ConnectionContext*)s->getData(0);
    if(ctx) delete ctx;
	if(i == -1)
	    listener.remove(s);
	else
	    listener._remove(i);
    if(clientCount) clientCount--;
    s->Close();
    delete s;
}

void PersServer::checkTimeouts()
{
	time_t timeBound = time(NULL) - timeout;
	int i = 1;
	smsc_log_debug(log, "Checking timeouts...");
	while(i < listener.count())
	{
		Socket* s = listener.get(i);
		ConnectionContext* ctx = (ConnectionContext*)s->getData(0);
		if(ctx->lastActivity  < timeBound)
		{
		    smsc_log_info(log, "Disconnecting inactive user by timeout");		
			removeSocket(s, i);
		}
		else
			i++;
	}
}

int PersServer::Execute()
{
	int lastTimeoutCheck = time(NULL);
    Multiplexer::SockArray read, write, err;

    if(isStopped()) return 1;

    if( sock.StartServer())
        smsc_log_warn(log, "Server socket can't start");

    while(!isStopped())
    {
        if(listener.canReadWrite(read, write, err, timeout * 1000))
        {
            if(isStopped())
                break;

            for(int i = 0; i < read.Count(); i++)
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
                        smsc_log_info(log, "Client connected: %s. Client count: %u", b, clientCount + 1);

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
                            sock1->setData(0, new ConnectionContext());
                            listener.addR(sock1);
                        }
                    } else
                        smsc_log_error(log, "accept failed: %s", strerror(errno));
                } 
                else
				{
					if(!((ConnectionContext*)read[i]->getData(0))->wantRead)	// in the case if we want to write and unexpected data arrive or EOF
						removeSocket(read[i]);
					else
	                    processReadSocket(read[i]);
				}
            }

            for(int i = 0; i < write.Count(); i++)
                processWriteSocket(write[i]);

            for(int i = 0; i <= err.Count() - 1; i++)
                if(err[i] == &sock)
                {
                    smsc_log_error(log, "Error on listen socket %d : %s", errno, strerror(errno));
                    listener.remove(&sock);
                    sock.Close();
                }
                else
                    removeSocket(err[i]);
        }
		
		if(lastTimeoutCheck + timeout < time(NULL))
		{
			checkTimeouts();
			lastTimeoutCheck = time(NULL);
		}
    }

    return 1;
}

}}
