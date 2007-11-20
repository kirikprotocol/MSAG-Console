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
    private String successRecord;

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
        successRecord=config.getProperty(prefix+".success");
        if(null == successRecord || "".equals(successRecord)){
            throw new InitializationException("no " + prefix + ".success" + " in config");
        }

    }

    protected void processState(State state) {
        String abonent;
        String body;
        synchronized (state) {
            if (!checkState(state)) {
                return;
            }
            if (logger.isDebugEnabled()) {
                logger.debug("Process state " + state);
            }
            state.addHistory(startHistoryRecord);
            body = state.getSourceMessage().getMessageString();
            abonent = state.getSourceMessage().getDestinationAddress();
            state.addHistory(endHistoryRecord);
            getStates().removeRequestState(state);
         }
        State abonentState = getStates().getRequestState("+"+abonent);
        String balance = body;
        synchronized (abonentState) {
            abonentState.addHistory(startHistoryRecord);
            abonentState.addResultObject("balance", balance);
            if (logger.isDebugEnabled()) {
                logger.debug("Added balance string:"+body+" to state:"+state);
            }
            abonentState.addHistory(endHistoryRecord);
            if(!"".equals(body)){
               abonentState.addHistory(successRecord);
            }
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
