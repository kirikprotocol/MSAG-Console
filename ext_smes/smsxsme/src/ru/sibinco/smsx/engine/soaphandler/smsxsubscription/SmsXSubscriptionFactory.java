package ru.sibinco.smsx.engine.soaphandler.smsxsubscription;

import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

/**
 * User: artem
 * Date: 28.07.2008
 */

public class SmsXSubscriptionFactory {
  private static SmsXSubscription handler;

  private static final Lock lock = new ReentrantLock();
  private static final Condition cond = lock.newCondition();

  private SmsXSubscriptionFactory() {
  }

  public static void init(String configDir) throws SOAPHandlerInitializationException {
    try {
      lock.lock();
      handler = new SmsXSubscriptionSoapHandler(configDir);
      cond.signal();
    } finally {
      lock.unlock();
    }
  }

  public static SmsXSubscription createGroupEdit() {
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
