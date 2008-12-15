package ru.sibinco.smsx.engine.soaphandler.groupsend;

import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: 17.07.2008
 */

public class GroupSendFactory {

  private static GroupSend handler;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  private GroupSendFactory() {
  }

  public static void init(String configDir) throws SOAPHandlerInitializationException {
    handler = new GroupSendSoapHandler(configDir);
    initLatch.countDown();
  }

  public static GroupSend createGroupSend() {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    return handler;
  }
}
