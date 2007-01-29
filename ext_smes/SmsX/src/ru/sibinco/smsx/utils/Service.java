package ru.sibinco.smsx.utils;

/**
 * User: artem
 * Date: Jul 31, 2006
 */

public abstract class Service extends Thread {
  private boolean started = false;
  private final Object starting = new Object();
  private final Object monitor = new Object();
  private final Object shutmonitor = new Object();

  private final org.apache.log4j.Category log;

  protected Service(org.apache.log4j.Category log) {
    this.log = log;
  }

  public void startService() {
    doBeforeStart();
    started = true;
    // Notify waiters
    synchronized(starting) {
      starting.notify();
    }
    start();
  }

  public synchronized void waitStarting() {
    if (!started) {
      synchronized(starting) {
        try {
          starting.wait();
        } catch (InterruptedException e) {
          log.error("Interrupted: ", e);
        }
      }
    }
  }

  public void stopService() {
    synchronized (shutmonitor) {
      synchronized (monitor) {
        started = false;
        monitor.notifyAll();
      }
      try {
        shutmonitor.wait();
      } catch (InterruptedException e) {
        log.warn(getName() + " shutting down exception:", e);
      }
    }
  }

  public boolean isStarted() {
    return started;
  }

  public final void run() {
    while(started) {
      iterativeWork();
    }

    synchronized (shutmonitor) {
      shutmonitor.notifyAll();
    }
  }

  public abstract void iterativeWork();

  protected void doBeforeStart() {
  }
}
