package ru.sibinco.smsx.engine.soaphandler.groupsend;

import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

/**
 * User: artem
 * Date: 17.07.2008
 */

public class GroupSendFactory {

  private static GroupSend handler;

  private static final Lock lock = new ReentrantLock();
  private static final Condition cond = lock.newCondition();

  private GroupSendFactory() {
  }

  public static void init(String configDir) throws SOAPHandlerInitializationException {
    try {
      lock.lock();
      handler = new GroupSendSoapHandler(configDir);
      cond.signal();
    } finally {
      lock.unlock();
    }
  }

  public static GroupSend createGroupSend() {
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
