package ru.sibinco.smsx.engine.smpphandler;

import ru.sibinco.smsc.utils.timezones.SmscTimezonesList;
import ru.sibinco.smsx.network.smppnetwork.SMPPMultiplexor;

/**
 * User: artem
 * Date: 03.07.2007
 */

public class SMPPHandlerFactory {

  public static SMPPHandler createSMPPHandler(String configDir, SmscTimezonesList timezonesList, SMPPMultiplexor multiplexor) {
    // Create handlers in chain
    final SenderSMPPHandler senderSMPPHandler = new SenderSMPPHandler(multiplexor);
    final CalendarSMPPHandler calendarSMPPHandler = new CalendarSMPPHandler(configDir, timezonesList, multiplexor);
    final SecretSMPPHandler secretSMPPHandler = new SecretSMPPHandler(configDir, multiplexor);
    final NickSMPPHandler nickSMPPHandler = new NickSMPPHandler(configDir, multiplexor);
    final RedirectSMPPHandler redirectSMPPHandler = new RedirectSMPPHandler(configDir, multiplexor);

    // Link handlers
    senderSMPPHandler.setNextHandler(calendarSMPPHandler);
    calendarSMPPHandler.setNextHandler(secretSMPPHandler);
    secretSMPPHandler.setNextHandler(nickSMPPHandler);
    nickSMPPHandler.setNextHandler(redirectSMPPHandler);

    // Return first handler
    return senderSMPPHandler;
  }
}
