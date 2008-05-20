package ru.sibinco.smsx.engine.service.nick;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: Sep 10, 2007
 */

public class NickServiceFactory {
  private static NickService impl = null;

  public static NickService getNickService(XmlConfig config, OutgoingQueue outQueue) {
    if (impl == null)
      impl = new NickServiceImpl(config, outQueue);
    return impl;
  }
}
