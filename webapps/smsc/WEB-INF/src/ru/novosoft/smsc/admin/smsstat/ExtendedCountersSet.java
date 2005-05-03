package ru.novosoft.smsc.admin.smsstat;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.TreeMap;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 24.10.2003
 * Time: 13:27:41
 * To change this template use Options | File Templates.
 */
public class ExtendedCountersSet extends CountersSet
{
    private TreeMap err=new TreeMap();

    public ExtendedCountersSet() {}
    public ExtendedCountersSet(long accepted, long rejected, long delivered,
                               long failed, long rescheduled, long temporal, long i, long o)
    {
        super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    }

    public void incError(int errcode, long count)
    {
        Integer key = new Integer(errcode);
        ErrorCounterSet set = (ErrorCounterSet)err.get(key);
        if (set == null) err.put(key, new ErrorCounterSet(errcode, count));
        else set.increment(count);
    }
    public void putErr(int errcode, ErrorCounterSet set)
    {
        err.put(new Integer(errcode), set);
    }
    public void addAllErr(Collection err)
    {
        err.addAll(err);
    }
    public Collection getErrors()
    {
        return err.values();
    }
    public TreeMap getErrorsMap()
    {
        return err;
    }

    public ErrorCounterSet getErr(int errcode)
    {
        return (ErrorCounterSet)err.get(new Integer(errcode));
    }
}
