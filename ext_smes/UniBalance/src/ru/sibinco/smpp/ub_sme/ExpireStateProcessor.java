package ru.sibinco.smpp.ub_sme;

import org.apache.log4j.Category;
import ru.sibinco.smpp.ub_sme.mg.MGRequestManager;
import ru.sibinco.smpp.ub_sme.mg.MGState;

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
    private MGRequestManager requestManager;

    public ExpireStateProcessor(MGState state, MGRequestManager requestManager) {
        this.state = state;
        this.requestManager = requestManager;
        this.timeout = requestManager.getExpireTime();
    }

    public void run() {
        synchronized (state.getExpireObject()) {
            try {
                state.getExpireObject().wait(timeout);
            } catch (InterruptedException e) {
                logger.warn("thread was interrupt", e);
            }
        }
        if (!state.isClosed()) {
            state.expire();
            requestManager.sendErrorSmsMessage(state);
            if (logger.isDebugEnabled()) {
                logger.debug("Expire state: " + state);
            }
        }

    }
}
