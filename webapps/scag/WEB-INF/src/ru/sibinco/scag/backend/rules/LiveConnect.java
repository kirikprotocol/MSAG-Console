package ru.sibinco.scag.backend.rules;

import ru.sibinco.lib.backend.util.config.Config;

import java.net.ServerSocket;
import java.net.Socket;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;

/**
 * User: dym
 * Date: 15.12.2006
 * Time: 12:56:40
 */
public class LiveConnect extends Thread {
  private String ping_host;
  private int ping_port;
  private int timeout;
  private ServerSocket serverSock;
  private Watcher watcher;
  private boolean isStopped = false;
  private Object shutSemaphore = new Object();

  public LiveConnect(RuleManager ruleManager, Config config) throws Exception{
    ping_host = config.getString("rule editor.ping_host");
    ping_port = (int)config.getInt("rule editor.ping_port");
    timeout = (int)config.getInt("rule editor.ping_timeout");
    serverSock = new ServerSocket(ping_port);
    watcher = new Watcher(ruleManager,timeout);
  }

  public void run() {
    while(!isStopped) {
      //System.out.println("**********LiveConnect run()");
      Socket sock = null;
      try {
        sock = serverSock.accept();
        //System.out.println("**********connection accepted "+ counter++);
        BufferedReader reader = new BufferedReader(new InputStreamReader(sock.getInputStream()));
        String ruleId;
        while ((ruleId = reader.readLine())!=null ) {
          //System.out.println("**********gettig value: " + ruleId);
          watcher.put(ruleId, new Long(System.currentTimeMillis()));
        }
       } catch(Exception e) {
        if (sock!=null) {
          try {
            sock.close();
          } catch (IOException io) {
            //io.printStackTrace();
          }
        }
        //e.printStackTrace();
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
      watcher.stopW();
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

  public int getTimeout() {
    return timeout;
  }

  public int getPingPort() {
    return ping_port;
  }

  public String getPingHost() {
    return ping_host;
  }

}
