package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import ru.sibinco.smsx.network.advertising.AdvertisingClient;

/**
 * User: artem
 * Date: Sep 5, 2007
 */

public class SmsXSenderFactory {

  private static SmsXSender sender;

  public static void init(String configDir, AdvertisingClient advertisingClient) {
    sender = new SmsXSenderHandler(configDir, advertisingClient);
  }

  public static SmsXSender createSmsXSenderHandler() {
    return sender;
  }
}
