package ru.sibinco.smpp.ub_sme;

import ru.sibinco.smpp.ub_sme.util.Utils;

import java.util.*;

/**
 * Created by pasha
 * Date: 08.11.2007
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public abstract class AbstractStateProcessor implements Runnable {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(SmeEngine.class);
    protected List requiredConditions = new LinkedList();
    protected String startHistoryRecord = "";
    protected String errorHistoryRecord = "";
    protected String endHistoryRecord = "";
    protected States states;
    protected SmeEngine smeEngine;
    protected int threadCount;
    protected int threadTimeout;
    protected String name;

    public AbstractStateProcessor(SmeEngine engine, States states) {
        this.smeEngine = engine;
        this.states = states;
    }

    public void init(Properties config, String name) {
        this.name = name;
        Enumeration enum = config.keys();
        while (enum.hasMoreElements()) {
            String key = (String) enum.nextElement();
            String start = "state.processor." + name + ".start.condition.";
            if (key.startsWith(start)) {
                String value = config.getProperty(key);
                StringTokenizer st = new StringTokenizer(value, ",");
                List list = new LinkedList();
                while (st.hasMoreElements()) {
                    list.add(st.nextToken());
                }
                requiredConditions.add(list);

            }
        }

        String prefix = "state.processor." + name;
        String history = prefix + ".start.history";
        startHistoryRecord = config.getProperty(history);
        if (null == startHistoryRecord || "".equals(startHistoryRecord)) {
            throw new InitializationException("no " + history + " in config");
        }
        history = prefix + ".error.history";
        errorHistoryRecord = config.getProperty(history);
        if (null == errorHistoryRecord || "".equals(errorHistoryRecord)) {
            throw new InitializationException("no " + history + " in config");
        }

        history = prefix + ".end.history";
        endHistoryRecord = config.getProperty(history);
        if (null == endHistoryRecord || "".equals(endHistoryRecord)) {
            throw new InitializationException("no " + history + "in config");
        }

        threadCount = Utils.getIntProperty(config, prefix + ".threads.count", 0);
        threadTimeout = Utils.getIntProperty(config, prefix + ".threads.timeout", 10);

        if (logger.isDebugEnabled()) {
            logger.debug(name + " processor init finished");
        }
    }

    public void run() {
        while (true) {
            try {
                for (int i = 0; i < states.size(); i++) {
                    State state = (State) states.get(i);
                    processState(state);
                }
            } catch (ArrayIndexOutOfBoundsException e) {
            }
            try {
                Thread.sleep(threadTimeout);
            } catch (InterruptedException e) {
                logger.warn("thread was interrupted", e);
            }
        }

    }
    protected abstract void processState(State state);
    protected boolean checkState(State state) {

        if (state.getHistory().contains(startHistoryRecord)) {
            return false;
        }

        if (requiredConditions.size() == 0) {
            return true;
        }
        for(int i=0;i<requiredConditions.size();i++){
            int j;
            List list = (List) requiredConditions.get(i);
            for (j = 0; j < list.size(); j++) {
                if (!state.getHistory().contains(list.get(j))) {
                    break;
                }
            }
            if (j == list.size()) {
                return true;
            }
        }
        return false;
    }

    public void startProcessorThreads() {
        for (int i = 0; i < threadCount; i++) {
            new Thread(this).start();
        }
        if (logger.isDebugEnabled()) {
            logger.debug("Started " + threadCount + " " + name + " with timeout " + threadTimeout);
        }
    }

    public String getStartHistoryRecord() {
        return startHistoryRecord;
    }

    public void setStartHistoryRecord(String startHistoryRecord) {
        this.startHistoryRecord = startHistoryRecord;
    }

    public String getErrorHistoryRecord() {
        return errorHistoryRecord;
    }

    public void setErrorHistoryRecord(String errorHistoryRecord) {
        this.errorHistoryRecord = errorHistoryRecord;
    }

    public String getEndHistoryRecord() {
        return endHistoryRecord;
    }

    public void setEndHistoryRecord(String endHistoryRecord) {
        this.endHistoryRecord = endHistoryRecord;
    }

    public SmeEngine getSmeEngine() {
        return smeEngine;
    }

    public States getStates() {
        return states;
    }
}
