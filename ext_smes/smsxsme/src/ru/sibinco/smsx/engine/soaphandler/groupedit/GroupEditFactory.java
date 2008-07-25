package ru.sibinco.smsx.engine.soaphandler.groupedit;

import ru.sibinco.smsx.engine.soaphandler.groupsend.GroupSend;
import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupEditFactory {
  
  private static GroupEdit handler;

  private static final Lock lock = new ReentrantLock();
  private static final Condition cond = lock.newCondition();

  private GroupEditFactory() {
  }

  public static void init(String configDir) throws SOAPHandlerInitializationException {
    try {
      lock.lock();
      handler = new GroupEditSoapHandler(configDir);
      cond.signal();
    } finally {
      lock.unlock();
    }
  }

  public static GroupEdit createGroupEdit() {
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
