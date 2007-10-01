package ru.sibinco.smsx.engine.service.nick;

import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class NickServiceFactory {
  private static NickService impl = null;

  public static NickService getNickService(String configDir, SMPPOutgoingQueue outQueue) {
    if (impl == null)
      impl = new NickServiceImpl(configDir, outQueue);
    return impl;
  }
}
