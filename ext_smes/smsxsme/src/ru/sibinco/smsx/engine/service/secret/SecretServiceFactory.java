package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.sme.smpp.OutgoingQueue;
import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: 05.07.2007
 */

public class SecretServiceFactory {

  private static SecretService impl;

  public static synchronized SecretService getSecretService(XmlConfig config, OutgoingQueue outQueue) {
    if (impl == null)
      impl = new SecretServiceImpl(config, outQueue);
    return impl;
  }
}
