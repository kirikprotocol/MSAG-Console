/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 11, 2002
 * Time: 1:31:30 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class HourCountersSet extends CountersSet
{
    private int hour=0;

    HourCountersSet(int hour) {
        this.hour = hour;
    }
    HourCountersSet(int a, int f, int r, int hour) {
        super(a, f, r); this.hour = hour;
    }
    HourCountersSet(CountersSet set, int hour) {
        super(set); this.hour = hour;
    }
    public int getHour() {
        return hour;
    }
}
