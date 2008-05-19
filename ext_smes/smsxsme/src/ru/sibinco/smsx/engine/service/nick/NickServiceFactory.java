package ru.sibinco.smsx.engine.service.nick;

import com.eyeline.sme.smpp.OutgoingQueue;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class NickServiceFactory {
  private static NickService impl = null;

  public static NickService getNickService(String configDir, OutgoingQueue outQueue) {
    if (impl == null)
      impl = new NickServiceImpl(configDir, outQueue);
    return impl;
  }
}
