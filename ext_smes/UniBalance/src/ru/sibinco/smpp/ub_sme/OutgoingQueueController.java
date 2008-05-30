package ru.sibinco.smpp.ub_sme;


public class OutgoingQueueController extends Thread {

  private final static org.apache.log4j.Category Logger = org.apache.log4j.Category.getInstance(OutgoingQueueController.class);

  private boolean started = false;
  private long pollingInterval = 10000L;
  private OutgoingQueue queue = null;
  private Object monitor = new Object();
  private Object shutmonitor = new Object();


  public OutgoingQueueController(OutgoingQueue queue, String name) throws InitializationException {
    setName(name);
    this.queue = queue;
    if (Logger.isInfoEnabled()) Logger.info(getName() + " use default polling interval: 10000 millis");
    if (Logger.isInfoEnabled()) Logger.info(getName() + " created.");
  }

  public OutgoingQueueController(OutgoingQueue queue, String name, long pollingInterval) {
    setName(name);
    if (pollingInterval < 10000)
      if (Logger.isInfoEnabled()) Logger.info(getName() + " use default polling interval: 10000 millis.");
      else {
        if (Logger.isInfoEnabled()) Logger.info(getName() + " use " + pollingInterval + " polling interval.");
        this.pollingInterval = pollingInterval;
      }
    this.queue = queue;
    if (Logger.isInfoEnabled()) Logger.info(getName() + " created.");
  }

  public void startService() {
    started = true;
    if (Logger.isInfoEnabled()) Logger.info(getName() + " started.");
    start();
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
        Logger.warn(getName() + " shutting down exception:", e);
      }
    }
  }

  public boolean isStarted() {
    return started;
  }

  public void run() {
    while (true) {
      queue.reschedule();
      synchronized (monitor) {
        if (!started) break;
        try {
          monitor.wait(pollingInterval);
        } catch (InterruptedException e) {
          if (Logger.isDebugEnabled()) Logger.debug(getName() + " was interrupted.", e);
        }
      }
    }
    synchronized (shutmonitor) {
      shutmonitor.notifyAll();
    }
  }
}
