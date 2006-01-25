package ru.sibinco.scag.backend.stat.stat;

import java.util.Date;
import java.util.Vector;
import java.util.Collection;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:54:45
 * To change this template use File | Settings | File Templates.
 */
public class DateCountersSet extends CountersSet {
    private Date date;

    private Vector byHours = new Vector(); // contains HourCountersSet

    DateCountersSet(Date date) {
        this.date = date;
    }

    public void addSMPPHourStat(HourCountersSet set) {
        byHours.addElement(set);
        super.incrementFullForSMPPTransport(set);
    }

    public void addHttpHourStat(HourCountersSet set) {
        byHours.addElement(set);
        super.incrementFullForHttpTransport(set);
    }

    public Collection getHourStat() {
        return byHours;
    }

    public Date getDate() {
        return date;
    }
}
