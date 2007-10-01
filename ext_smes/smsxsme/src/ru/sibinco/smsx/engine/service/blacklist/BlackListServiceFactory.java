package ru.sibinco.smsx.engine.service.blacklist;

/**
 * User: artem
 * Date: 10.07.2007
 */

public class BlackListServiceFactory {

  private static BlackListService impl;

  public static BlackListService getBlackListService(String configDir) {
    if (impl == null)
      impl = new BlackListServiceImpl(configDir);
    return impl;
  }
}
