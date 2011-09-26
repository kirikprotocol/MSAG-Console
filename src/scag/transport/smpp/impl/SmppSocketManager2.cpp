#include "scag/util/RelockMutexGuard.h"
#include "SmppSocketManager2.h"
#include <arpa/inet.h>

namespace {

uint32_t getNetworkAddress( const struct in_addr& addr )
{
#if defined(linux) || defined(__MACH__)
    return addr.s_addr;
#else
    return addr.S_un.S_addr;
#endif
}

}


namespace scag2 {

using util::RelockMutexGuard;

namespace transport {
namespace smpp {


void SmppSocketManager::addWhiteIp( const char* dotted )
{
    if (!dotted) return;
    struct in_addr dst;
    if ( 1 != inet_pton( AF_INET, dotted, &dst ) ) {
        smsc_log_warn(log,"address %s cannot be added", dotted);
    }
    RelockMutexGuard mg(statMutex_);
    const uint32_t netaddr = getNetworkAddress(dst);
    uint8_t* isWhite = whiteList_.GetPtr(netaddr);
    if ( !isWhite ) {
        whiteList_.Insert(netaddr,1);
        mg.Unlock();
        smsc_log_info(log,"whitelisted ip %s is added", dotted);
    }
}


bool SmppSocketManager::registerSocket(SmppSocket* sock)
{
    SmppSocketPtr sockPtr(sock);
    do {

        const uint32_t netaddr = getNetworkAddress(sock->getPeerAddress().sin_addr);
        int cpi = -1;
        unsigned regc;
        uint8_t* isWhite = 0;
        {
            RelockMutexGuard mg(statMutex_);
            isWhite = whiteList_.GetPtr(netaddr);
            IpLimit* l = 0;
            if ( isWhite ) {
                // in white list - no limits
            } else if ( sock->getType() == etService ) {
                l = iphash_.GetPtr( netaddr );
                if ( !l ) {
                    // no limits yet
                    iphash_.Insert(netaddr, IpLimit());
                    l = iphash_.GetPtr(netaddr);
                } else if ( l->connectionCount() > connectionsPerIp_ ) {
                    mg.Unlock();
                    smsc_log_warn( log, "Connection %s is dropped: max connect limit: %u",
                                   sock->getPeer(), connectionsPerIp_ );
                    break;
                } else if ( l->lastFailure() ) {
                    const time_t now = time(0);
                    const int tmo = int(now - l->lastFailure());
                    if ( tmo < int(failTimeout_) ) {
                        mg.Unlock();
                        smsc_log_warn( log, "Connection %s is dropped: last failure was %d seconds ago, tmo=%d",
                                       sock->getPeer(), tmo, failTimeout_ );
                        break;
                    } else {
                        // reset failure
                        l->setLastFailure(0);
                    }
                }
            }
            if (l) cpi = l->addConnection();
            regc = ++registeredConnections_;
        }

        // now we have checked that socket is not limited, and collected some stats
        // proceed with assigning r/w
        RelockMutexGuard mg(mtx);
        sock->setInterfaces(queue,reg);
        for (int i=0;i<readers.Count();i++)
        {
            if (unsigned(readers[i]->getSocketsCount()) < socketsPerThread_ )
            {
                // if (l) l->addConnection();
                sock->setSocketManager(this);
                readers[i]->addSocket(sockPtr);
                writers[i]->addSocket(sockPtr);
                // ++registeredConnections_;
                const unsigned rc = readers[i]->getSocketsCount();
                mg.Unlock();
                sock = 0;
                smsc_log_info(log,"Reusing reader/writer (%d), conn(perIp/total)=%d/%d",rc,cpi,regc);
                break;
            }
        }

        // we have not found any not fully filled reader
        if ( sock ) {
            if ( unsigned(readers.Count()) >= maxReaderCount_ ) {
                // we have to drop connection
                const unsigned rc = readers.Count();
                mg.Unlock();
                
                // remove from statistics
                {
                    RelockMutexGuard smg(statMutex_);
                    --registeredConnections_;
                    if ( isWhite || sock->getType() != etService ) {
                        // no need to find iplimits
                    } else {
                        IpLimit* l = iphash_.GetPtr(netaddr);
                        if (!l) {
                            smg.Unlock();
                            smsc_log_error(log,"logic error: ip(%x) has been removed from iphash", netaddr);
                        } else {
                            l->setLastFailure(time(0));
                        }
                    }
                }
                
                smsc_log_warn(log,"Connection %s is dropped: too many readers/writers: %d",
                              sock->getPeer(), rc );
                break;
            }
            sock->setSocketManager(this);
            SmppReader* rd = new SmppReader();
            SmppWriter* wr = new SmppWriter(*this);
            readers.Push(rd);
            writers.Push(wr);
            rd->addSocket(sockPtr);
            wr->addSocket(sockPtr);
            tp.startTask(rd);
            tp.startTask(wr);
            const unsigned rc = readers.Count();
            mg.Unlock();
            // sock->release();
            sock = 0;
            smsc_log_info(log,"Creating new reader/writer (%d), conn(perIp/total)=%d/%d", rc, cpi, regc );
        }
    } while ( false ); // fake loop

    if ( sock ) {
        sock->disconnect();
        // sock->release();
        return false;
    }
    return true;
}

void SmppSocketManager::unregisterSocket(SmppSocket* sock)
{
    unsigned rc;
    int cpi;
    do {
        RelockMutexGuard mg(statMutex_);
        rc = --registeredConnections_;
        if (sock->getType()==etSmsc) {
            mg.Unlock();
            cpi = -1;
            conn->reportSmscDisconnect(sock->getSystemId());
            break;
        } else if ( sock->getType() == etService ) {
            const uint32_t netaddr = getNetworkAddress(sock->getPeerAddress().sin_addr);
            if (!netaddr) { break; }
            uint8_t* isWhite = whiteList_.GetPtr(netaddr);
            if ( isWhite ) {
                cpi = -1;
                break;
            }
            IpLimit* l = iphash_.GetPtr(netaddr);
            if ( !l ) {
                mg.Unlock();
                smsc_log_error( log,"logic error: failed to receive iplimit structure on %s", sock->getPeer());
                // FIXME: temporary
                smsc_log_fatal( log,"temporary abort will follow");
                ::abort();
                break;
            }
            cpi = l->removeConnection();
            if ( sock->hasBindFailed() ) {
                l->setLastFailure(time(0));
                // } else if ( l->connectionCount() == 0 ) {
                // iphash_.Delete(netaddr);
            }
        } else {
            cpi = -1;
        }
    } while ( false );
    smsc_log_info(log,"connection %s unregistered, perIP/total=%d/%u", sock->getPeer(), cpi, rc);
}

void SmppSocketManager::shutdown()
{
  if (conn) conn->shutdown();
  if (acc) {
      acc->Stop();
      // wait until acc is stopped
      while ( !acc->isStopped() ) {
          Thread::Yield();
      }
  }
  tp.shutdown();
  readers.Empty();
  writers.Empty();
}


}//smpp
}//transport
}//scag
