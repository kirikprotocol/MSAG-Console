#include "PersSocketServer.h"
#include "util/Exception.hpp"

namespace scag { namespace pers {

using smsc::util::Exception;

#define MAX_PACKET_SIZE 100000

PersSocketServer::PersSocketServer(const char *persHost_, int persPort_, int maxClientCount_,
                                    int timeout_, int transactTimeout_)
    : log(Logger::getInstance("server")), persHost(""), persPort(0), isStopping(false)
{
    persHost = persHost_;
    persPort = persPort_;
    maxClientCount = maxClientCount_;
    clientCount = 0;
	timeout = timeout_;
    transactTimeout = transactTimeout_;
}

PersSocketServer::~PersSocketServer()
{
    listener.remove(&sock);
    sock.Close();
    
    while(listener.count())
        removeSocket(listener.get(0));
}

void PersSocketServer::InitServer()
{
    if(sock.InitServer(persHost.c_str(), persPort, 10, 1, true))
        throw Exception("Failed to init socket server by host: %s, port: %d", persHost.c_str(), persPort);

    smsc_log_info(log, "Socket server is inited by host: %s, port: %d", persHost.c_str(), persPort);

    if(!listener.addR(&sock))
        throw Exception("Failed to init PersSocketServer");
    bindToCP();
}

void PersSocketServer::processReadSocket(Socket* s)
{
    long k;
	ConnectionContext* ctx = (ConnectionContext*)s->getData(0);
    SerialBuffer& sb = ctx->inbuf;

    if(sb.GetSize() < PACKET_LENGTH_SIZE)
    {
      int j = s->Read(tmp_buf, PACKET_LENGTH_SIZE - sb.GetSize());

        smsc_log_debug(log, "read(len) %u bytes from %p", j, s);
        if(j > 0)
        {
            sb.Append(tmp_buf, j);
            if(sb.GetSize() >= PACKET_LENGTH_SIZE)
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
            if(j)  { 
              smsc_log_debug(log, "Error: %s(%d)", strerror(errno), errno);
            }
//          if(errno != EWOULDBLOCK)
                removeSocket(s);
            return;
        }
    }
    if(sb.GetSize() >= PACKET_LENGTH_SIZE)
    {
        int j = ctx->packetLen - sb.GetSize();
        j = s->Read(tmp_buf, j > 1024 ? 1024 : j);

        smsc_log_debug(log, "read %u bytes from %p", j, s);

        if(j > 0) {
          sb.Append(tmp_buf, j);
        }
        else if(errno != EWOULDBLOCK)
        {
            if(j) smsc_log_debug(log, "Error: %s(%d)", strerror(errno), errno);
            removeSocket(s);
            return;
        }
        if(sb.GetSize() >= ctx->packetLen)
        {
            smsc_log_debug(log, "read from socket:%p len=%d, data=%s", s, sb.length(), sb.toString().c_str());
			//ctx->outbuf.Empty();
            ctx->inbuf.SetPos(PACKET_LENGTH_SIZE);
            if(processPacket(*ctx))
            {
                if (ctx->inbuf.GetPos() == ctx->inbuf.GetSize()) {
                  smsc_log_debug(log, "processReadSocket : Empty inbuf in socket %p", s);
                  ctx->inbuf.Empty();
                }
    			ctx->lastActivity = time(NULL);	
                if (ctx->batch && ctx->batch_cmd_count > 0) {
                  smsc_log_debug(log, "processReadSocket : batch not complite, add socket %p as READ", s);
                  listener.addR(s);
                  ctx->wantRead = true;
                  return;
                }
                ctx->batch = false;
                ctx->batch_cmd_count = 0;
                ctx->outbuf.SetPos(0);
                if (ctx->outbuf.length() > PACKET_LENGTH_SIZE) {
                  smsc_log_debug(log, "processReadSocket : add socket %p as READ/WRITE outbuf length = %d",
                                  s, ctx->outbuf.length());
                  listener.addRW(s);
                  //ctx->wantRead = false; // indicate that we want write and read is only for EOF signalling
                } else {
                  ctx->outbuf.SetPos(PACKET_LENGTH_SIZE);
                  smsc_log_debug(log, "processReadSocket : add socket %p as READ", s);
                  listener.addR(s);
                  //ctx->wantRead = true;
                }
                ctx->wantRead = true;
            }
            else {
              removeSocket(s);                
            }
        }
    }
}

void PersSocketServer::processWriteSocket(Socket* s)
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
        smsc_log_debug(log, "written to socket: len=%d, data=%s", j, sb.toString().c_str());
        ctx->inbuf.Empty();
        ctx->outbuf.Empty();
        ctx->outbuf.SetPos(PACKET_LENGTH_SIZE);
		ctx->lastActivity = time(NULL);
        listener.addR(s);
		ctx->wantRead = true;
    }
}

void PersSocketServer::removeSocket(Socket* s, int i)
{
  if (!s || !clientCount) {
    return;
  }
    char b[256];
    s->GetPeer(b);
    ConnectionContext *ctx = (ConnectionContext*)s->getData(0);
    if(ctx)
    {
        onDisconnect(*ctx);
        delete ctx;
    }
    if(clientCount) clientCount--;
    smsc_log_info(log, "Socket disconnected: %s. Client count: %u", b, clientCount);    
	if(i == -1)
	    listener.remove(s);
	else
	    listener._remove(i);
    s->Close();
    delete s;
    s = NULL;
}

void PersSocketServer::checkTimeouts(time_t curTime)
{
	time_t timeBound = curTime - timeout;
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

int PersSocketServer::Execute()
{
	int lastTimeoutCheck = time(NULL);
    int lastTransactTimeoutCheck = lastTimeoutCheck;
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
            int readCount = read.Count();
            for(int i = 0; i < readCount; i++)
            {
                Socket *readSocket = read[i];
                if(readSocket == &sock)
                {
                    sockaddr_in addrin;
                    socklen_t sz = sizeof(addrin);
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
                            sock1->setData(0, new ConnectionContext(sock1));
                            listener.addR(sock1);
                        }
                    } else
                        smsc_log_error(log, "accept failed: %s", strerror(errno));
                } 
                else
				{
					if(!((ConnectionContext*)readSocket->getData(0))->wantRead) {	// in the case if we want to write and unexpected data arrive or EOF
                      smsc_log_error(log, "unexpected data arrive in %p ", readSocket);
                      removeSocket(readSocket);
                    }
					else {
                      processReadSocket(readSocket);
                    }
				}
            }

            int writeCount = write.Count();
            for(int i = 0; i < writeCount; i++)
                processWriteSocket(write[i]);

            int errCount = err.Count();
            for(int i = 0; i <= errCount - 1; i++)
                if(err[i] == &sock)
                {
                    smsc_log_error(log, "Error on listen socket %d : %s", errno, strerror(errno));
                    listener.remove(&sock);
                    sock.Close();
                }
                else
                    removeSocket(err[i]);
        }
        time_t curTime = time(NULL);
        if (transactTimeout && (lastTransactTimeoutCheck + transactTimeout < curTime)) {
          checkTransactionsTimeouts(curTime);
          lastTransactTimeoutCheck = curTime;
        }
		
		if(lastTimeoutCheck + timeout < curTime)
		{
			checkTimeouts(curTime);
			lastTimeoutCheck = curTime;
		}
    }

    return 1;
}

}}
