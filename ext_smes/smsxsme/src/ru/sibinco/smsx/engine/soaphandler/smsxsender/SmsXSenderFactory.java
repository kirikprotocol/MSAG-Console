package ru.sibinco.smsx.engine.soaphandler.smsxsender;

import ru.sibinco.smsx.utils.operators.OperatorsList;
import ru.sibinco.smsx.network.advertising.AdvertisingClient;

/**
 * User: artem
 * Date: Sep 5, 2007
 */

public class SmsXSenderFactory {

  private static SmsXSender sender;

  public static void init(String configDir, OperatorsList operatorsList, AdvertisingClient advertisingClient) {
    sender = new SmsXSenderHandler(configDir, operatorsList, advertisingClient);
  }

  public static SmsXSender createSmsXSenderHandler() {
    return sender;
  }
}
