package org.gjt.sp.jedit;

import java.net.ServerSocket;
import java.net.Socket;
import java.io.IOException;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 27.09.2006
 * Time: 16:07:10
 * To change this template use File | Settings | File Templates.
 */
public class LiveConnect extends Thread {
  private int port;
  private ServerSocket serverSock;
  private boolean isStopped = false;
  private Object shutSemaphore = new Object();


  public LiveConnect() {
    super("Live connect");
    //System.out.println("**********LiveConnect constructor");
    try {
      serverSock = new ServerSocket(0);
      port = serverSock.getLocalPort();
      //System.out.println("**********port = " + port);
    } catch (Exception e) {
      e.printStackTrace();
    }
  }

  public int getPingPort() {
    return port;
  }

  public void run() {
    while(!isStopped) {
      //System.out.println("**********LiveConnect run()");
      Socket sock = null;
      try {
        sock = serverSock.accept();
        //System.out.println("**********connection accepted");
        int pingValue = sock.getInputStream().read();
        //System.out.println("**********gettig value: " + pingValue);
        sock.getOutputStream().write(pingValue);
      } catch(Exception e) {
        if (sock!=null) {
          try {
            sock.close();
          } catch (IOException io) {}
        }
        e.printStackTrace();
      }
    }
    synchronized (shutSemaphore) {
      shutSemaphore.notifyAll();
    }
  }

  public void stopLC() {
    //System.out.println("**********LiveConnect stop()");
    synchronized (shutSemaphore) {
      isStopped = true;
      try {
        serverSock.close();
      } catch(Exception e) {
          e.printStackTrace();
      }
      try {
        shutSemaphore.wait();
      } catch(InterruptedException ie) {}
    }
  }
}
