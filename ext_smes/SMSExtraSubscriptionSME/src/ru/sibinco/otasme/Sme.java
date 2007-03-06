package ru.sibinco.otasme;

import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.sibinco.otasme.engine.SmeEngine;
import ru.sibinco.otasme.network.*;
import ru.sibinco.otasme.utils.Utils;

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

    SmeProperties.init();

    new SmeEngine().startService();

    // connect multiplexor
    try {
      multiplexor.connect();
    } catch (SMPPException e) {
      throw new InitializationException("Exception occured during establishing connection in multiplexor.");
    }
    // run message sender
    messageSender.startService();
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
}
