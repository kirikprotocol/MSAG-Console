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
    private Date date;

    private Vector byHours = new Vector(); // contains HourCountersSet

    DateCountersSet(Date date) {
        this.date = date;
    }
    public void addHourStat(HourCountersSet set) {
        byHours.addElement(set);
        super.increment(set);
    }
    public Collection getHourStat() {
        return byHours;
    }
    public Date getDate() {
        return date;
    }
}
