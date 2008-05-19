package ru.sibinco.smsx.engine.service.secret;

import com.eyeline.sme.smpp.OutgoingQueue;

/**
 * User: artem
 * Date: 05.07.2007
 */

public class SecretServiceFactory {

  private static SecretService impl;

  public static synchronized SecretService getSecretService(String configDir, OutgoingQueue outQueue) {
    if (impl == null)
      impl = new SecretServiceImpl(configDir, outQueue);
    return impl;
  }
}
