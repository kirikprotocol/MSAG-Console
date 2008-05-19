package ru.sibinco.smsx.engine.service.sender;

import com.eyeline.sme.smpp.OutgoingQueue;


/**
 * User: artem
 * Date: 06.07.2007
 */

public class SenderServiceFactory {
  private static SenderService impl;

  public static SenderService getSenderService(String configDir, OutgoingQueue outQueue) {
    if (impl == null)
      impl = new SenderServiceImpl(configDir, outQueue);
    return impl;
  }
}
