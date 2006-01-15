package ru.sibinco.scag.backend.stat.stat;

import java.util.Vector;
import java.util.Collection;
import java.util.TreeMap;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:58:44
 * To change this template use File | Settings | File Templates.
 */
public class ExtendedCountersSet extends CountersSet {

    private TreeMap err = new TreeMap();

    public ExtendedCountersSet() {
    }

    public ExtendedCountersSet(long accepted, long rejected, long delivered,
                               long gw_rejected, long failed) {
        super(accepted, rejected, delivered, gw_rejected, failed);
    }

    public ExtendedCountersSet(long request, long requestRejected, long response,
                               long responseRejected, long delivered, long failed) {
        super(request, requestRejected, response, responseRejected, delivered, failed);
    }

    public ExtendedCountersSet(long accepted, long rejected, long delivered,
                               long gw_rejected, long failed, long billingOk,
                               long billingFailed, long recieptOk, long recieptFailed) {
        super(accepted, rejected, delivered, gw_rejected, failed, billingOk, billingFailed, recieptOk, recieptFailed);
    }

    public ExtendedCountersSet(long request, long requestRejected, long response,
                               long responseRejected, long delivered, long failed,
                               long billingOk, long billingFailed) {
        super(request, requestRejected, response, responseRejected, delivered, failed, billingOk, billingFailed);
    }

    public void incError(int errcode, long count) {
        Integer key = new Integer(errcode);
        ErrorCounterSet set = (ErrorCounterSet) err.get(key);
        if (set == null) err.put(key, new ErrorCounterSet(errcode, count));
        else set.increment(count);
    }

    public void putErr(int errcode, ErrorCounterSet set) {
        err.put(new Integer(errcode), set);
    }

    public void addAllErr(Collection err) {
        err.addAll(err);
    }

    public Collection getErrors() {
        return err.values();
    }

    public TreeMap getErrorsMap() {
        return err;
    }

    public ErrorCounterSet getErr(int errcode) {
        return (ErrorCounterSet) err.get(new Integer(errcode));
    }
}
