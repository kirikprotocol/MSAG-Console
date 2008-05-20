package ru.sibinco.smsx.engine.service.blacklist;

import com.eyeline.utils.config.xml.XmlConfig;

/**
 * User: artem
 * Date: 10.07.2007
 */

public class BlackListServiceFactory {

  private static BlackListService impl;

  public static BlackListService getBlackListService(XmlConfig config) {
    if (impl == null)
      impl = new BlackListServiceImpl(config);
    return impl;
  }
}
