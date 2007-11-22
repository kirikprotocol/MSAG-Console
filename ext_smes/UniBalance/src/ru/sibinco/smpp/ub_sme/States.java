package ru.sibinco.smpp.ub_sme;

import java.util.LinkedList;
import java.util.ArrayList;

/**
 * Created by pasha
 * Date: 12.11.2007
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class States extends ArrayList {
    private final static org.apache.log4j.Category logger = org.apache.log4j.Category.getInstance(States.class);

    synchronized public void addRequestState(State state) {
        this.add(state);
        if (logger.isDebugEnabled())
            logger.debug(state.getSourceMessage().getSourceAddress() + " request state added.states count:"+size());
    }

    synchronized public State getRequestState(String abonent) {
        int index = this.indexOf(new RequestStateIndex(abonent));
        if (index == -1) {
            if (logger.isDebugEnabled())
                logger.debug(abonent + " request state not found.states count:"+size());
            return null;
        }
        return (State) this.get(index);
    }

    synchronized public State extractRequestState(String abonent) {
        int index = this.indexOf(new RequestStateIndex(abonent));
        if (index == -1) {
            if (logger.isDebugEnabled())
                logger.debug(abonent + " request state not found.");
            return null;
        }
        return (State) this.remove(index);
    }

    synchronized public void removeRequestState(State state) {
        if (logger.isDebugEnabled()) {
            logger.debug("Remove state: " + state);
        }
        remove(state);
    }

}
