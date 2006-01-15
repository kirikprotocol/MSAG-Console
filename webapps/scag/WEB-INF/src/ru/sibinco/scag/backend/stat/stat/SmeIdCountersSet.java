package ru.sibinco.scag.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 14:02:01
 * To change this template use File | Settings | File Templates.
 */
public class SmeIdCountersSet extends ExtendedCountersSet implements Comparable {
    protected String smeid;
    public int providerId = -1;

    public SmeIdCountersSet(String smeid) {
        this.smeid = smeid;
    }


    public SmeIdCountersSet(long accepted, long rejected, long delivered,
                            long gw_rejected, long failed, String smeid) {
        super(accepted, rejected, delivered, gw_rejected, failed);
        this.smeid = smeid;
    }

    public SmeIdCountersSet(long request, long requestRejected, long response,
                               long responseRejected, long delivered, long failed, String smeid) {
        super(request, requestRejected, response, responseRejected, delivered, failed);
        this.smeid = smeid;
    }

    public int compareTo(Object o) {
        if (o == null || smeid == null || !(o instanceof SmeIdCountersSet)) return -1;
        return smeid.compareTo(((SmeIdCountersSet) o).smeid);
    }

    public String getSmeid() {
        return smeid;
    }

    public int getProviderId() {
        return providerId;
    }

    public void setProviderId(int providerId) {
        this.providerId = providerId;
    }
}
