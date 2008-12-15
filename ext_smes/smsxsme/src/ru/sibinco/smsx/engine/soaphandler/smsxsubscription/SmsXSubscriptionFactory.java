package ru.sibinco.smsx.engine.soaphandler.smsxsubscription;

import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;

import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: 28.07.2008
 */

public class SmsXSubscriptionFactory {
  private static SmsXSubscription handler;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  private SmsXSubscriptionFactory() {
  }

  public static void init(String configDir) throws SOAPHandlerInitializationException {
    handler = new SmsXSubscriptionSoapHandler(configDir);
    initLatch.countDown();
  }

  public static SmsXSubscription createGroupEdit() {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    return handler;
  }
}
