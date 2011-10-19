package ru.novosoft.smsc.admin.topmon;

import ru.novosoft.smsc.util.applet.SnapBufferReader;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.SocketException;


public class TopServerHandler extends Thread {

  private static final org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(TopServerHandler.class);

  private Socket sock;
  private Socket smscSock;
  private final TopServer server;
  private final SnapBufferReader inbuf = new SnapBufferReader();

  public TopServerHandler(Socket sock, TopServer server) throws IOException {
    this.sock = sock;
    this.server = server;
    logger.debug("Connecting to topmon data socket " + server.getSmscHost() + ":" + server.getTopPort());
    initSmscConn();
  }

  protected void initSmscConn() throws IOException {
    smscSock = new Socket(server.getSmscHost(), server.getTopPort());
    smscSock.setSoLinger(true, 5);
    logger.debug("Connected to performance data socket " + server.getSmscHost() + ":" + server.getTopPort());
  }


  protected void _handle(DataOutputStream os) throws IOException{
    InputStream is = null;
    try {
      is = smscSock.getInputStream();
      TopSnap snap = new TopSnap();
      while (!isStopping) {
        readSnap(is, snap);
        snap.write(os);
        os.flush();
      }
    }finally{
      if (is != null)
        try {
          is.close();
        } catch (Exception ignored) {
        }
    }
  }

  protected void handle() {
    DataOutputStream os = null;
    try {
      os = new DataOutputStream(sock.getOutputStream());
      _handle(os);
    } catch (SocketException e) {
      logger.debug("TopMon: Client " + sock.getInetAddress().getHostAddress() + " disconnected");
    } catch (IOException e) {
      logger.warn("I/O error occured for " + sock.getInetAddress().getHostAddress(), e);
    } catch (Exception e) {
      logger.error("Unexpected error occured for " + sock.getInetAddress().getHostAddress(), e);
    } finally {
      if (os != null)
        try {
          os.close();
        } catch (Exception ignored) {
        }
      if (sock != null)
        try {
          sock.close();
        } catch (Exception ignored) {
        }
      if (smscSock != null)
        try {
          smscSock.close();
        } catch (Exception ignored) {
        }
    }
  }

  public void run() {
    handle();
    server.removeRunner(this);
    synchronized (shutSemaphore) {
      shutSemaphore.notifyAll();
    }
  }

  void readSnap(InputStream istream, TopSnap snap) throws IOException {
    int len = inbuf.readNetworkInt(istream);
    inbuf.fill(istream, len);
    snap.init(inbuf);
  }

  private final Object shutSemaphore = new Object();
  protected boolean isStopping = false;

  public void shutdown() {
    synchronized (shutSemaphore) {
      isStopping = true;
      try {
        smscSock.close();
      } catch (IOException ignored) {
      }
      try {
        sock.close();
      } catch (IOException ignored) {
      }
      try {
        shutSemaphore.wait();
      } catch (InterruptedException ignored) {
      }
    }
  }

}
