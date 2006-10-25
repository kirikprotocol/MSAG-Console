package ru.sibinco.calendarsme.network;

import ru.sibinco.calendarsme.InitializationException;
import ru.sibinco.calendarsme.utils.Utils;
import ru.sibinco.calendarsme.utils.Service;

import java.util.Properties;


public class OutgoingQueueController extends Service {

  private final static org.apache.log4j.Category Log = org.apache.log4j.Category.getInstance(OutgoingQueueController.class);

  private final long pollingInterval;
  private final OutgoingQueue queue;
  private final Object monitor = new Object();

  public OutgoingQueueController(final Properties config, final OutgoingQueue queue) {
    super(Log);

    setName("OutgoingQueueController");

    pollingInterval = Utils.loadLong(config, "outgoing.queue.controller.polling.interval");
    if (pollingInterval < 10000)
      throw new InitializationException("OutgoingQueueController: polling interval < 10000");

    this.queue = queue;
    Log.info(getName() + " created.");
  }

  public void iterativeWork() {
    queue.reschedule();
    synchronized (monitor) {
      try {
        monitor.wait(pollingInterval);
      } catch (InterruptedException e) {
        Log.debug(getName() + " was interrupted.", e);
      }
    }
  }
}
