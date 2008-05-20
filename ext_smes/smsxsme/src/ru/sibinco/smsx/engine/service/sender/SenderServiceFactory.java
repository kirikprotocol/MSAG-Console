package ru.sibinco.smsx.engine.service.sender;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;


/**
 * User: artem
 * Date: 06.07.2007
 */

public class SenderServiceFactory {
  private static SenderService impl;

  public static SenderService getSenderService(XmlConfig config, OutgoingQueue outQueue) {
    if (impl == null)
      impl = new SenderServiceImpl(config, outQueue);
    return impl;
  }
}
