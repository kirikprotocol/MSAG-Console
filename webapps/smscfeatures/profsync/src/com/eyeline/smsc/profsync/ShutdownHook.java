package com.eyeline.smsc.profsync;

/**
 * Created by Serge Lugovoy
 * Date: Dec 5, 2006
 * Time: 4:00:51 PM
 */
public class ShutdownHook extends Thread {
  public ShutdownHook() {
  }

  public void run() {
    ProfileSync.shutdown();
  }
}
