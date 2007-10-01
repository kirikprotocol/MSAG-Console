package ru.sibinco.smsx.engine;

import com.eyeline.sme.utils.worker.IterativeWorker;
import org.apache.log4j.Category;
import ru.sibinco.smsx.engine.smpphandler.SMPPHandler;
import ru.sibinco.smsx.network.smppnetwork.SMPPIncomingQueue;
import ru.sibinco.smsx.network.smppnetwork.SMPPTransportObject;

/**
 * User: artem
 * Date: 03.07.2007
 */

class SMPPWorker extends IterativeWorker {

  private static final Category log = Category.getInstance(SMPPWorker.class);

  private final SMPPIncomingQueue inQueue;
  private final SMPPHandler handler;

  SMPPWorker(SMPPIncomingQueue inQueue, SMPPHandler handler) {
    super(log);
    this.inQueue = inQueue;
    this.handler = handler;
  }

  public void iterativeWork() {
    final SMPPTransportObject inObj = inQueue.getInObj();
    if (inObj != null)
      handler.handleIncomingObject(inObj);
  }

  protected void stopCurrentWork() {
    inQueue.notifyWaiters();
  }
}
