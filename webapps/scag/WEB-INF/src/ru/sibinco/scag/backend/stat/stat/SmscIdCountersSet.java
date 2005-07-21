/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.stat.stat;

/**
 * The <code>SmscIdCountersSet</code> class represents
 * <p><p/>
 * Date: 19.07.2005
 * Time: 18:49:26
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SmscIdCountersSet extends ExtendedCountersSet implements Comparable {

    protected String smscid;
    public int providerId = -1;

    public SmscIdCountersSet(String smscId) {
        this.smscid = smscId;
    }

    public SmscIdCountersSet(long accepted, long rejected, long delivered,
                             long gw_rejected, long failed, String smscId) {
        super(accepted, rejected, delivered, gw_rejected, failed);
        this.smscid = smscId;
    }

    public String getSmscid() {
        return smscid;
    }

    public int compareTo(Object o) {
        if (o == null || smscid == null || !(o instanceof SmscIdCountersSet)) return -1;
        return smscid.compareTo(((SmscIdCountersSet) o).smscid);
    }

    public int getProviderId() {
        return providerId;
    }

    public void setProviderId(int providerId) {
        this.providerId = providerId;
    }
}
