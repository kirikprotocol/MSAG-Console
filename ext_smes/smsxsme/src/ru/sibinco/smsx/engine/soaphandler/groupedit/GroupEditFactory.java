package ru.sibinco.smsx.engine.soaphandler.groupedit;

import com.eyeline.sme.smpp.OutgoingQueue;
import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: 25.07.2008
 */

public class GroupEditFactory {
  
  private static GroupEdit handler;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  private GroupEditFactory() {
  }

  public static void init(String configDir, OutgoingQueue outQueue) throws SOAPHandlerInitializationException {
    handler = new GroupEditSoapHandler(configDir, outQueue);
    initLatch.countDown();
  }

  public static GroupEdit createGroupEdit() {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    return handler;
  }
}
