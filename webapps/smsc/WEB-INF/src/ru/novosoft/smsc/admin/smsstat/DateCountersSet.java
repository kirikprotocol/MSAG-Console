/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:37:26 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

import java.util.Date;
import java.util.Vector;
import java.util.Collection;

public class DateCountersSet extends CountersSet
{
    private int startHour = 0;
    private Date date;

    private Vector byHours = new Vector(); // contains CountersSet

    DateCountersSet(Date date) {
        this.date = date;
    }
    DateCountersSet(Date date, int hour) {
        this.date = date; this.startHour = hour;
    }
    public void addHourStat(CountersSet set) {
        byHours.addElement(set);
        System.out.println("ADD HOUR a:"+set.accepted+
                            " f:"+set.finalized+
                            " r:"+set.rescheduled);
        super.increment(set);
    }
    public Collection getHourStat() {
        return byHours;
    }
    public Date getDate() {
        return date;
    }
    public int getStartHour() {
        return startHour;
    }
}
