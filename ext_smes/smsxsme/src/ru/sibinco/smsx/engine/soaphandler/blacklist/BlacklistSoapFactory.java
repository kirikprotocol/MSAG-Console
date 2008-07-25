package ru.sibinco.smsx.engine.soaphandler.blacklist;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

/**
 * User: artem
 * Date: Sep 5, 2007
 */

public class BlacklistSoapFactory {

  private static BlacklistSoap handler;

  private static final Lock lock = new ReentrantLock();
  private static final Condition cond = lock.newCondition();

  private BlacklistSoapFactory() {
  }

  public static void init(String configDir) {
    try {
      lock.lock();
      handler = new BlacklistSoapHandler();
      cond.signal();
    } finally {
      lock.unlock();
    }

  }

  public static BlacklistSoap createBlacklistSoapHandler() {
    try {
      lock.lock();
      if (handler == null)
        cond.await();
      return handler;
    } catch (InterruptedException e) {
      e.printStackTrace();
      return null;
    } finally {
      lock.unlock();
    }
  }
}
