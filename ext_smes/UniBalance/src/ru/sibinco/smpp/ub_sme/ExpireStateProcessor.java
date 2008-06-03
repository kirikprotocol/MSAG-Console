package ru.sibinco.smpp.ub_sme;

import org.apache.log4j.Category;

/**
 * User: pasha
 * Date: 28.11.2007
 * Time: 10:45:04
 */
public class ExpireStateProcessor implements Runnable {
  private static Category logger = Category.getInstance(ExpireStateProcessor.class);

  private State state;
  private SmeEngine smeEngine;

  public ExpireStateProcessor(State state, SmeEngine smeEngine) {
    this.state = state;
    this.smeEngine = smeEngine;
  }

  public void run() {
    synchronized (state.getExpireObject()) {
      try {
        state.getExpireObject().wait(smeEngine.getStateExpireTime());
      } catch (InterruptedException e) {
        logger.warn("thread was interrupt", e);
      }
    }
    if (!state.isClosed()) {
      state.expire();
      smeEngine.sendErrorSmsMessage(state);
    }

  }
}
