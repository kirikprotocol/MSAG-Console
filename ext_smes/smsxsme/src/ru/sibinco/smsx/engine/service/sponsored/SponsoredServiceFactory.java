package ru.sibinco.smsx.engine.service.sponsored;

import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;

/**
 * User: artem
 * Date: 05.07.2007
 */

public class SponsoredServiceFactory {
  private static SponsoredService impl;

  public static synchronized SponsoredService getSponsoredService(String configDir, SMPPOutgoingQueue outQueue) {
    if (impl == null)
      impl = new SponsoredServiceImpl(configDir, outQueue);
    return impl;
  }
}
