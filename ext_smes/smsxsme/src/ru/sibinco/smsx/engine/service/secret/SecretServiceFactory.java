package ru.sibinco.smsx.engine.service.secret;

import ru.sibinco.smsx.network.smppnetwork.SMPPOutgoingQueue;

/**
 * User: artem
 * Date: 05.07.2007
 */

public class SecretServiceFactory {

  private static SecretService impl;

  public static synchronized SecretService getSecretService(String configDir, SMPPOutgoingQueue outQueue) {
    if (impl == null)
      impl = new SecretServiceImpl(configDir, outQueue);
    return impl;
  }
}
