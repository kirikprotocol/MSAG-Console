package ru.sibinco.smsx.engine.soaphandler.blacklist;

/**
 * User: artem
 * Date: Sep 5, 2007
 */

public class BlacklistSoapFactory {

  private static BlacklistSoap handler;

  public static void init(String configDir) {
    handler = new BlacklistSoapHandler();
  }

  public static BlacklistSoap createBlacklistSoapHandler() {
    return handler;
  }
}
