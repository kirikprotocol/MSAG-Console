/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 5, 2002
 * Time: 2:32:02 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

import java.util.Vector;
import java.util.Collection;

public class Statistics
{
    private CountersSet total = new CountersSet();
    private Vector byDates = new Vector(); // contains DateCountersSet
    private Vector byError = new Vector(); // contains ErrorCounterSet
    private Vector bySmeId = new Vector(); // contains SmeIdCountersSet
    private Vector byRouteId = new Vector(); // contains RouteIdCountersSet

    public CountersSet getTotal() {
        return total;
    }

    public void addDateStat(DateCountersSet set) {
        byDates.addElement(set);
        total.increment(set);
    }
    public Collection getDateStat() {
        return byDates;
    }

    public void addErrorStat(ErrorCounterSet set) {
        byError.addElement(set);
    }
    public Collection getErrorStat() {
        return byError;
    }

    public void addSmeIdStat(SmeIdCountersSet set) {
        bySmeId.addElement(set);
    }
    public Collection getSmeIdStat() {
        return bySmeId;
    }

    public void addRouteIdStat(RouteIdCountersSet set) {
        byRouteId.addElement(set);
    }
    public Collection getRouteIdStat() {
        return byRouteId;
    }
}
