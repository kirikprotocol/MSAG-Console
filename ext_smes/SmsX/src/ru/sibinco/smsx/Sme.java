package ru.sibinco.smsx;

import ru.aurorisoft.smpp.Multiplexor;
import ru.aurorisoft.smpp.SMPPException;
import ru.aurorisoft.smpp.SQLStatisticsCollectorWithSourceAndMessageAndConnectionName;
import ru.aurorisoft.smpp.StatisticsCollector;
import ru.sibinco.smsx.engine.SmeEngine;
import ru.sibinco.smsx.network.*;
import ru.sibinco.smsx.utils.Utils;

import java.util.HashSet;
import java.util.Set;
import java.util.StringTokenizer;

/**
 * User: artem
 * Date: Jul 27, 2006
 */

public final class Sme {

  Sme() throws InitializationException{
    SmeProperties.init();

    final java.util.Properties config = Utils.loadConfig("sme.properties");

    final IncomingQueue inQueue = new IncomingQueue(config);

    final OutgoingQueue outQueue = new OutgoingQueue(config);

    new OutgoingQueueController(config, outQueue);

    final Multiplexor multiplexor = initMultiplexor(config, inQueue);

    ResponseListenerImpl responseListener = new ResponseListenerImpl(outQueue, inQueue);
    multiplexor.setResponseListener(responseListener);

    final MessageSender messageSender = new MessageSender(config, outQueue, multiplexor);

    responseListener.setSendMonitor(messageSender.getSendMonitor());

    new SmeEngine(inQueue, outQueue, multiplexor).startService();

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

  private static Multiplexor initMultiplexor(final java.util.Properties config, final IncomingQueue inQueue) {
    final Multiplexor multiplexor = new Multiplexor();

    try {
      final StatisticsCollector statisticsCollector = new SQLStatisticsCollectorWithSourceAndMessageAndConnectionName();
      statisticsCollector.init(config, "stats.collector");

      multiplexor.init(Utils.loadConfig("network.properties"), "", statisticsCollector);

      final Set smscAddresses = new HashSet();
      final StringTokenizer st = new StringTokenizer(Utils.loadString(config, "smsc.addresses"), ",");
      while (st.hasMoreTokens())
        smscAddresses.add(st.nextToken());

      multiplexor.setMessageListener(new MessageListenerImpl(config, multiplexor, inQueue, smscAddresses));
    } catch (SMPPException e) {
      e.printStackTrace();
      throw new InitializationException(e);
    }

    return multiplexor;
  }

  public static void main(String args[]) {
    new Sme();
  }
}
