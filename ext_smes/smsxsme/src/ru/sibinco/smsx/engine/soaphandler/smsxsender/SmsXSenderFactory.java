package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import ru.sibinco.smsx.network.advertising.AdvertisingClient;
import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

/**
 * User: artem
 * Date: Sep 5, 2007
 */

public class SmsXSenderFactory {

  private static SmsXSender sender;

  private static final Lock lock = new ReentrantLock();
  private static final Condition cond = lock.newCondition();

  private SmsXSenderFactory() {
  }

  public static void init(String configDir, AdvertisingClient advertisingClient) throws SOAPHandlerInitializationException {
    try {
      lock.lock();
      sender = new SmsXSenderHandler(configDir, advertisingClient);
      cond.signal();
    } finally {
      lock.unlock();
    }

  }

  public static SmsXSender createSmsXSenderHandler() {
    try {
      lock.lock();
      if (sender == null)
        cond.await();
      return sender;
    } catch (InterruptedException e) {
      e.printStackTrace();
      return null;
    } finally {
      lock.unlock();
    }

  }
}
