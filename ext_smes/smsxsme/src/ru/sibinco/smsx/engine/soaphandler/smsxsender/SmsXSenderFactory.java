package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import ru.sibinco.smsx.engine.soaphandler.SOAPHandlerInitializationException;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;

import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: Sep 5, 2007
 */

public class SmsXSenderFactory {

  private static SmsXSenderHandler sender;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  private SmsXSenderFactory() {
  }

  public static void init(String configDir, AdvertisingClient advertisingClient) throws SOAPHandlerInitializationException {
    sender = new SmsXSenderHandler(configDir, advertisingClient);
    initLatch.countDown();
  }

  public static SmsXSenderHandler createSmsXSenderHandler() {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    return sender;
  }
}
