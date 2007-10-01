package ru.sibinco.smsx.engine.service.sender;

import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;

/**
 * User: artem
 * Date: 06.07.2007
 */

public class SenderServiceFactory {
  private static SenderService impl;

  public static SenderService getSenderService(String configDir, SMPPOutgoingQueue outQueue) {
    if (impl == null)
      impl = new SenderServiceImpl(configDir, outQueue);
    return impl;
  }
}
