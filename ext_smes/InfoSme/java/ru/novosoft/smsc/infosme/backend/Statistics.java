package ru.novosoft.smsc.infosme.backend;

import java.util.Vector;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 30.09.2003
 * Time: 13:21:38
 * To change this template use Options | File Templates.
 */

public class Statistics
{
    private CountersSet counters = new CountersSet();

    public CountersSet getCounters() {
        return counters;
    }
    public void addDateStat(DateCountersSet set) {
        counters.increment(set);
    }
}
