package ru.novosoft.smsc.admin.smsstat;

import java.util.Collection;
import java.util.TreeMap;
import java.util.Iterator;

public class ExtendedCountersSet extends CountersSet {
    private TreeMap errors = new TreeMap();

    public ExtendedCountersSet() {
    }

    public ExtendedCountersSet(long accepted, long rejected, long delivered,
                               long failed, long rescheduled, long temporal, long i, long o) {
        super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    }

    public void incError(int errcode, long count) {
        Integer key = new Integer(errcode);
        ErrorCounterSet set = (ErrorCounterSet) errors.get(key);
        if (set == null)
            errors.put(key, new ErrorCounterSet(errcode, count));
        else
            set.increment(count);
    }

    public void addAllErr(Collection err) {
        for (Iterator i = err.iterator(); i.hasNext();) {
            ErrorCounterSet set = (ErrorCounterSet) i.next();
            if (set != null) errors.put(new Integer(set.errcode), set);
        }
    }

    public Collection getErrors() {
        return errors.values();
    }
}
