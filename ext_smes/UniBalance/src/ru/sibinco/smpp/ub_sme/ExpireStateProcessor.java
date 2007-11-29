package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.mg.MGState;
import org.apache.log4j.Category;

/**
 * User: pasha
 * Date: 28.11.2007
 * Time: 10:45:04
 */
public class ExpireStateProcessor implements Runnable {
    private static org.apache.log4j.Category logger =
            Category.getInstance(ExpireStateProcessor.class);
    private MGState state;
    private int timeout;

    public ExpireStateProcessor(MGState state) {
        this.state = state;
        this.timeout = Sme.getSmeEngine().getExpireTime();
    }

    public void run() {
        synchronized (state.getExpireObject()){
            try {
                wait(timeout);
            } catch (InterruptedException e) {
                 logger.warn("thread was interrupt", e);
            }
        }

        if (!state.isClosed()) {
            state.expire();
            Sme.getSmeEngine().sendErrorSmsMessage(state);
            if(logger.isDebugEnabled()){
                logger.debug("Expire state: "+state);
            }
        }
    }
}
