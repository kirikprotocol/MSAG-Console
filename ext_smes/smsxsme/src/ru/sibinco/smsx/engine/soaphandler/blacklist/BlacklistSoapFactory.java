package ru.sibinco.smsx.engine.soaphandler.blacklist;

import java.util.concurrent.CountDownLatch;

/**
 * User: artem
 * Date: Sep 5, 2007
 */

public class BlacklistSoapFactory {

  private static BlacklistSoap handler;

  private static final CountDownLatch initLatch = new CountDownLatch(1);

  private BlacklistSoapFactory() {
  }

  public static void init(String configDir) {
    handler = new BlacklistSoapHandler();
    initLatch.countDown();
  }

  public static BlacklistSoap createBlacklistSoapHandler() {
    try {
      initLatch.await();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    return handler;
  }
}
