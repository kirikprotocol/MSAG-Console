package ru.sibinco.smpp.ub_sme;

import ru.aurorisoft.smpp.Message;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;

/**
 * Created by pasha
 * Date: 09.11.2007
 * Copyright (c)
 * EyeLine Communications
 * All rights reserved.
 */
public class State {
    protected List history = new LinkedList();
    protected List historyTimes = new LinkedList();
    protected HashMap map = new HashMap();
    private final static DateFormat dateFormat = new SimpleDateFormat("HH:mm:ss.SSS");
    protected boolean closed = false;
    protected long abonentRequestTime;
    protected long abonentResponseTime;
    protected Message sourceMessage;

    public State(Message msg, long abonentRequestTime) {
        sourceMessage = msg;
        this.abonentRequestTime = abonentRequestTime;
    }


    public boolean isUssdSessionClosed() {
        return closed;
    }

    public void setUssdSessionClosed(boolean closed) {
        this.closed = closed;
    }


    public Message getSourceMessage() {
        return sourceMessage;
    }

    public void setSourceMessage(Message sourceMessage) {
        this.sourceMessage = sourceMessage;
    }

    public long getAbonentRequestTime() {
        return abonentRequestTime;
    }


    public void addHistory(String event) {
        history.add(event);
        historyTimes.add(new Long(System.currentTimeMillis()- abonentRequestTime));
    }

    public List getHistory() {
        return history;
    }

    public void addResultObject(String s, Object o) {
        map.put(s, o);
    }


    public Object getResultObject(String key) {
        return map.get(key);
    }

    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append("Abonent request: ");
        String date;
        synchronized (this) {
            synchronized (dateFormat) {
                date = dateFormat.format(new Date(abonentRequestTime));
            }
                sb.append(date);
                sb.append(";History: ");
                for (int i = 0; i < history.size(); i++) {
                    sb.append(history.get(i));
                    sb.append(" ");
                    sb.append(((Long) historyTimes.get(i)).longValue());
                    sb.append("ms; ");
                }
        }
        return sb.toString();
    }

    public long getAbonentResponseTime() {
        return abonentResponseTime;
    }

    public void setAbonentResponseTime(long abonentResponseTime) {
        this.abonentResponseTime = abonentResponseTime;
    }
}
