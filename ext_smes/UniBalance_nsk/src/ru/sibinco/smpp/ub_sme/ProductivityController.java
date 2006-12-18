package ru.sibinco.smpp.ub_sme;

import org.apache.log4j.Category;

import java.util.LinkedList;
import java.util.List;
import java.util.Properties;

import ru.sibinco.smpp.ub_sme.InitializationException;
import ru.sibinco.smpp.ub_sme.ProductivityControllable;

/**
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class ProductivityController extends Thread {
  private final static Category logger = Category.getInstance(ProductivityController.class);

  private long pollingInterval = 10000L;

  private boolean stop = false;

  private Object monitor = new Object();
  private Object shutMonitor = new Object();
  private List controlObjects = new LinkedList();

  public void init(Properties config) throws InitializationException {
    logger.debug("init(..)");

    try {
      pollingInterval = Long.parseLong(config.getProperty("productivity.controller.polling.interval", Long.toString(pollingInterval)));
    } catch (NumberFormatException e) {
      throw new InitializationException("Bad value of productivity.controller.polling.interval: " + config.getProperty("productivity.controller.polling.interval"));
    }
    logger.info("productivity.controller.polling.interval: " + pollingInterval);

    logger.debug("init(..) finished");
  }

  public void addControlObject(ProductivityControllable obj){
    obj.setEventsCounterEnabled(true);
    controlObjects.add(obj);
  }

  /**
   * Starts the thread
   */
  public void startService() {
    logger.debug("startService()");
    if(!logger.isInfoEnabled()){
      logger.warn("Logger INFO is disabled. Couldn't start ProductivityController");
      return;
    }
    stop = false;
    start();
  }

  /**
   * Stops the thread
   */
  public void stopService() {
    synchronized (shutMonitor) {
      synchronized (monitor) {
        logger.debug("stopService()");
        stop = true;
        monitor.notifyAll();
      }
      try {
        shutMonitor.wait();
      } catch (InterruptedException e) {
        logger.warn(getName() + " shutting down exception: " + e, e);
      }
    }
  }

  public void run() {
    while (!stop) {
      for (int i = 0; i < controlObjects.size(); i++) {
        ProductivityControllable object = (ProductivityControllable) controlObjects.get(i);
        logger.info(object.getName()+": "+(1000L*object.getEventsCount()/(System.currentTimeMillis()-object.getCounterStartTime()))+" / sec.");
        object.resetEventsCounter();
      }
      synchronized (monitor) {
        try {
          monitor.wait(pollingInterval);
        } catch (InterruptedException e) {
          logger.debug(getName() + " was interrupted.", e);
        }
      }
    }
    synchronized (shutMonitor) {
      shutMonitor.notifyAll();
    }
  }

}
