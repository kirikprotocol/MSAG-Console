package ru.sibinco.smppgw.perfmon;

import ru.sibinco.lib.backend.util.SnapBufferReader;

import java.net.Socket;
import java.net.SocketException;
import java.io.*;

public class PerfServerRunner extends Thread {
  private org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(PerfServerRunner.class);

  Socket sock;
  Socket smscSock;
  PerfServer server;
  SnapBufferReader inbuf = new SnapBufferReader();

  public PerfServerRunner(Socket sock, PerfServer server)
      throws IOException {
    this.sock = sock;
    this.server = server;
    logger.debug("Connecting to performance data socket " + server.getSmscHost() + ":" + server.getPerfPort());
    smscSock = new Socket(server.getSmscHost(), server.getPerfPort());
    smscSock.setSoLinger(true, 5);
    logger.debug("Connected to performance data socket " + server.getSmscHost() + ":" + server.getPerfPort());
  }

  public void run() {
    DataOutputStream os = null;
    InputStream is = null;
    try {
      os = new DataOutputStream(sock.getOutputStream());
      is = smscSock.getInputStream();
//            snapGenerator(os);
      PerfSnap snap = new PerfSnap();
      while (!isStopping) {
        readSnap(is, snap);
        snap.write(os);
        os.flush();
      }
    } catch (SocketException e) {
      logger.debug("Client " + sock.getInetAddress().getHostAddress() + " disconnected");
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

  protected void readSnap(InputStream istream, PerfSnap snap)
      throws IOException {
    int len = inbuf.readNetworkInt(istream);
    inbuf.fill(istream, len - 4);
    snap.init(inbuf);
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
