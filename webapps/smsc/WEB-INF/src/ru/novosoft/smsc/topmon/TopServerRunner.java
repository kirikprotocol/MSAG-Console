package ru.novosoft.smsc.topmon;

import ru.novosoft.smsc.util.SnapBufferReader;

import java.net.Socket;
import java.net.SocketException;
import java.io.IOException;
import java.io.DataOutputStream;
import java.io.InputStream;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 12:16:42
 */
public class TopServerRunner  extends Thread {
  private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

  Socket sock;
  Socket smscSock;
  TopServer server;
  SnapBufferReader inbuf = new SnapBufferReader();

  public TopServerRunner(Socket sock, TopServer server) throws IOException {
    this.sock = sock;
    this.server = server;
    logger.debug("Connecting to topmon data socket " + server.getSmscHost() + ":" + server.getTopPort());
    smscSock = new Socket(server.getSmscHost(), server.getTopPort());
    smscSock.setSoLinger(true, 5);
    logger.debug("Connected to performance data socket " + server.getSmscHost() + ":" + server.getTopPort());
  }

  public void run() {
    DataOutputStream os = null;
    InputStream is = null;
    try {
      os = new DataOutputStream(sock.getOutputStream());
      is = smscSock.getInputStream();
      TopSnap snap = new TopSnap();
      while (!isStopping) {
        readSnap(is, snap);
        snap.write(os);
        os.flush();
      }
    } catch (SocketException e) {
      logger.debug("TopMon: Client " + sock.getInetAddress().getHostAddress() + " disconnected");
    } catch (IOException e) {
      logger.warn("I/O error occured for " + sock.getInetAddress().getHostAddress(), e);
    } catch (Exception e) {
      logger.error("Unexpected error occured for " + sock.getInetAddress().getHostAddress(), e);
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (Exception ee) {
        }
      ;
      if (os != null)
        try {
          os.close();
        } catch (Exception ee) {
        }
      ;
      if (sock != null)
        try {
          sock.close();
        } catch (Exception ee) {
        }
      ;
      if (smscSock != null)
        try {
          smscSock.close();
        } catch (Exception ee) {
        }
      ;
    }
    server.removeRunner(this);
    synchronized (shutSemaphore) {
      shutSemaphore.notifyAll();
    }
  }

  protected void readSnap(InputStream istream, TopSnap snap) throws IOException {
    int len = inbuf.readNetworkInt(istream);
    inbuf.fill(istream, len);
    snap.init(inbuf);
    if( logger.isDebugEnabled() ) {
      logger.debug("got snap: "+snap.smeCount);
      for( int i = 0; i < snap.smeCount; i++) {
        StringBuffer sb = new StringBuffer(128);
        for( int k = 0; k < SmeSnap.COUNTERS_NUM; k++ ) {
          sb.append(snap.smeSnap[i].speed[k]).append('/').append(snap.smeSnap[i].avgSpeed[k]).append(' ');
        }
        logger.debug("     sme: "+snap.smeSnap[i].smeId+" "+sb.toString() );
      }
      for( int i = 0; i < snap.errCount; i++) {
        logger.debug("     err: "+snap.errSnap[i].code+" "+snap.errSnap[i].count+"/"+snap.errSnap[i].avgCount );
      }
    }
  }

  Object shutSemaphore = new Object();
  boolean isStopping = false;

  public void shutdown() {
    synchronized (shutSemaphore) {
      isStopping = true;
      try {
        shutSemaphore.wait();
      } catch (InterruptedException e) {
      }
    }
  }

}
