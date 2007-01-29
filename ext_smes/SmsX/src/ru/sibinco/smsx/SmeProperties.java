package ru.sibinco.smsx;

import ru.sibinco.smsx.utils.Utils;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

/**
 * User: artem
 * Date: 10.11.2006
 */

public class SmeProperties {

  public static String CALENDAR_ADDRESS;
  public static String SECRET_ADDRESS;
  public static String SPONSORED_ADDRESS;
  public static String REDIRECTOR_ADDRESS;

  private static final HashMap ruleMessages = new HashMap();
  private static final HashSet serviceAddresses = new HashSet();

  private SmeProperties() {}

  public static String getRuleMessage(String number) {
    return (String)ruleMessages.get(number);
  }

  public static Iterator getServiceAddresses() {
    return serviceAddresses.iterator();
  }

  public static void init() {
    final java.util.Properties config = Utils.loadConfig("sme.properties");
    config.setProperty("config.path", "sme.properties");

    CALENDAR_ADDRESS = Utils.loadString(config, "service.address.calendar");
    SECRET_ADDRESS = Utils.loadString(config, "service.address.secret");
    SPONSORED_ADDRESS = Utils.loadString(config, "service.address.sponsored");
    REDIRECTOR_ADDRESS = Utils.loadString(config, "service.address.redirector");

    serviceAddresses.add(CALENDAR_ADDRESS);
    serviceAddresses.add(SECRET_ADDRESS);
    serviceAddresses.add(SPONSORED_ADDRESS);
    serviceAddresses.add(REDIRECTOR_ADDRESS);

    for(Iterator iter = config.keySet().iterator(); iter.hasNext();){
      final String key = (String)iter.next();
      if (key.startsWith("rule.message.")) {
        final String serviceAddress = key.substring("rule.message.".length());
        ruleMessages.put(serviceAddress, config.getProperty(key));
      }
    }
  }
}