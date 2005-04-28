package ru.novosoft.smsc.admin.smsstat;

/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 10, 2002
 * Time: 4:37:57 PM
 * To change this template use Options | File Templates.
 */

public class ErrorCounterSet
{
    public int errcode;
    public long counter;

    ErrorCounterSet(int err, long count)
    {
        errcode = err; counter = count;
    }
    protected void increment(long count)
    {
        this.counter += count;
    }
}
