/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:37:00 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.smsstat;

public class CountersSet
{
    public int accepted = 0;
    public int finalized = 0;
    public int rescheduled = 0;

    CountersSet() {}
    CountersSet(CountersSet set) {
        accepted = set.accepted;
        finalized = set.finalized;
        rescheduled = set.rescheduled;
    }
    CountersSet(int a, int f, int r) {
        accepted = a; finalized = f; rescheduled =r;
    }

    void increment(CountersSet set) {
        accepted += set.accepted;
        finalized += set.finalized;
        rescheduled += set.rescheduled;
    }
}
