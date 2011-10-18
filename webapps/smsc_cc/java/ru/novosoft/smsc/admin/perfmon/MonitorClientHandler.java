package ru.novosoft.smsc.admin.perfmon;

import ru.novosoft.smsc.util.InetAddress;
import ru.novosoft.smsc.util.applet.SnapBufferReader;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.net.SocketException;

public class MonitorClientHandler extends Thread {

  private final org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(this.getClass());

  private Socket sock;
  private Socket smscSock;
  private PerfServer server;
  private final SnapBufferReader inbuf = new SnapBufferReader();

  private final boolean support64Bit;

  MonitorClientHandler() {
    support64Bit = false;
  }

  public MonitorClientHandler(Socket sock, PerfServer server, boolean support64Bit)
      throws IOException {
    this.sock = sock;
    this.server = server;
    InetAddress smscAddress = server.getSmscAddress();
    logger.debug("Connecting to performance data socket " + smscAddress);
    smscSock = new Socket(smscAddress.getHost(), smscAddress.getPort());
    smscSock.setSoLinger(true, 5);
    this.support64Bit = support64Bit;
    logger.debug("Connected to performance data socket " + smscAddress);
  }

  protected void handle() {
    DataOutputStream os = null;
    InputStream is = null;
    try {
      os = new DataOutputStream(sock.getOutputStream());
      is = smscSock.getInputStream();
      PerfSnap snap = new PerfSnap(support64Bit);
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
        } catch (Exception ignored) {
        }
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
    if (server != null) {
      server.removeHandler(this);
    }
    synchronized (shutSemaphore) {
      shutSemaphore.notifyAll();
    }
  }

  void readSnap(InputStream istream, PerfSnap snap)
      throws IOException {
    int len = inbuf.readNetworkInt(istream);
    inbuf.fill(istream, len - 4);
    snap.init(inbuf);
  }

  private final Object shutSemaphore = new Object();
  boolean isStopping = false;

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
