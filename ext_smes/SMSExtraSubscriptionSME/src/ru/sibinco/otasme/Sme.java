package ru.sibinco.otasme;

import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.otasme.engine.SmeEngine;
import ru.sibinco.otasme.network.*;
import ru.sibinco.otasme.utils.Utils;
import ru.sibinco.otasme.utils.Service;
import ru.sibinco.smsc.utils.smscenters.SmsCentersList;
import ru.sibinco.smsc.utils.smscenters.SmsCentersListException;

import java.util.HashSet;
import java.util.Properties;
import java.util.Set;
import java.util.StringTokenizer;

/**
 * User: artem
 * Date: 26.10.2006
 */

public final class Sme {

  public static final IncomingQueue inQueue;
  public static final OutgoingQueue outQueue;
  public static final Multiplexor multiplexor;
  public static final MessageSender messageSender;

  static {
    final Properties config = Utils.loadConfig("sme.properties");
    inQueue = new IncomingQueue(config);
    outQueue = new OutgoingQueue(config, inQueue);

    new OutgoingQueueController(config, outQueue);

    multiplexor = initMultiplexor(config, inQueue);

    ResponseListenerImpl responseListener = new ResponseListenerImpl(outQueue, inQueue);
    multiplexor.setResponseListener(responseListener);

    messageSender = new MessageSender(config, outQueue, multiplexor);

    responseListener.setSendMonitor(messageSender.getSendMonitor());

    config.clear();
  }

  private Sme() throws InitializationException{
    try {
      SmeProperties.init();

      final SmsCentersList smsCenters = new SmsCentersList(SmeProperties.Templates.SMSCENTERS_XML, SmeProperties.Templates.ROUTES_XML);

      // Start SMS centers reloader
      new SMSCentersReloader(smsCenters).startService();

      // Start Sme engine
      new SmeEngine(smsCenters).startService();

      // Connect multiplexor
      multiplexor.connect();

      // Run message sender
      messageSender.startService();

    } catch (SMPPException e) {
      throw new InitializationException("Exception occured during establishing connection in multiplexor.");
    } catch (SmsCentersListException e) {
      throw new InitializationException(e);
    }
  }

  private static Multiplexor initMultiplexor(final Properties config, final IncomingQueue inQueue) {
    final Multiplexor multiplexor = new Multiplexor();

    try {
      multiplexor.init(Utils.loadConfig("network.properties"), "");

      final Set smscAddresses = new HashSet();
      final StringTokenizer st = new StringTokenizer(Utils.loadString(config, "smsc.addresses"), ",");
      while (st.hasMoreTokens())
        smscAddresses.add(st.nextToken());

      multiplexor.setMessageListener(new MessageListenerImpl(config, multiplexor, inQueue, smscAddresses));
    } catch (SMPPException e) {
      throw new InitializationException(e);
    }

    return multiplexor;
  }

  public static void main(String args[]) {
    new Sme();
  }

  private static class SMSCentersReloader extends Service {
    private boolean errorOccured = false;
    private final SmsCentersList smsCenters;

    SMSCentersReloader(SmsCentersList smsCenters) {
      super(null);
      this.smsCenters = smsCenters;
    }

    public synchronized void iterativeWork() {
      try {
        wait((!errorOccured) ? SmeProperties.Templates.RELOAD_INTERVAL : 15000);
      } catch (InterruptedException e) {
        e.printStackTrace();
//        log.error("Interrupted", e);
      }

      try {
        smsCenters.reload();
      } catch (SmsCentersListException e) {
        e.printStackTrace();
//        log.error("Can't reload SMS centers", e);
      }
    }
  }
}
