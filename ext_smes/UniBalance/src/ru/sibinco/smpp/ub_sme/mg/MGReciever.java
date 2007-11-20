package ru.sibinco.smpp.ub_sme.mg;

import ru.sibinco.smpp.ub_sme.*;

import java.util.Properties;

/**
 * Created by pasha
 * Date: 08.11.2007
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class MGReciever extends AbstractStateProcessor {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(MGReciever.class);
    private String mgAddress;

    public MGReciever(SmeEngine engine, States states) {
        super(engine, states);
    }


    public void init(Properties config, String name) {
        super.init(config, name);
        String prefix = "state.processor." + name;
        mgAddress = config.getProperty(prefix + ".mg_address");
        if (null == mgAddress || "".equals(mgAddress)) {
            throw new InitializationException("no " + prefix + ".mg_address" + " in config");
        }

    }

    protected void processState(State state) {
        String abonent;
        String body;
        synchronized (state) {
            if (!checkState(state)) {
                return;
            }

            getStates().removeRequestState(state);
            body = state.getSourceMessage().getMessageString();
            abonent = state.getSourceMessage().getDestinationAddress();
            if (logger.isDebugEnabled()) {
                logger.debug("Process state ");
                logger.debug("Message from mg "+body);
            }
        }
        State abonentState = getStates().getRequestState(abonent);
        String balance = body;
        synchronized (abonentState) {
            abonentState.addResultObject("balance", balance);
            abonentState.addHistory(endHistoryRecord);
        }

    }

    protected boolean checkState(State state) {
        synchronized (state) {
            if (!mgAddress.equals(state.getSourceMessage().getSourceAddress())) {
                return false;
            }
        }
        return super.checkState(state);
    }
}
