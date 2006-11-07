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

  static {
    final Properties config = Utils.loadConfig("sme.properties");
    inQueue = new IncomingQueue(config);
    outQueue = new OutgoingQueue(config, inQueue);
  }

  private Sme() throws InitializationException{
    final Properties config = Utils.loadConfig("sme.properties");

    new OutgoingQueueController(config, outQueue);

    final Multiplexor multiplexor = initMultiplexor(config, inQueue);

    ResponseListenerImpl responseListener = new ResponseListenerImpl(outQueue, inQueue);
    multiplexor.setResponseListener(responseListener);

    final MessageSender messageSender = new MessageSender(config, outQueue, multiplexor);

    responseListener.setSendMonitor(messageSender.getSendMonitor());

    new SmeEngine().startService();

    // connect multiplexor
    try {
      multiplexor.connect();
    } catch (SMPPException e) {
      throw new InitializationException("Exception occured during establishing connection in multiplexor.");
    }
    // run message sender
    messageSender.startService();

    config.clear();
  }

  private static Multiplexor initMultiplexor(final Properties config, final IncomingQueue inQueue) {
    final Multiplexor multiplexor = new Multiplexor();

    try {
      multiplexor.init(Utils.loadConfig("network.properties"), "");

      final Set smscAddresses = new HashSet();
      final StringTokenizer st = new StringTokenizer(Utils.loadString(config, "smsc.addresses"), ",");
      while (st.hasMoreTokens())
        smscAddresses.add(st.nextToken());

      multiplexor.setMessageListener(new MessageListenerImpl(multiplexor, inQueue, smscAddresses));
    } catch (SMPPException e) {
      throw new InitializationException(e);
    }

    return multiplexor;
  }

  public static void main(String args[]) {
    new Sme();
  }
}
