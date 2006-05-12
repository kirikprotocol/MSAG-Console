package ru.sibinco.scag.backend.stat.stat;

import ru.sibinco.lib.backend.util.Functions;

import java.util.Date;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.08.2004
 * Time: 14:40:39
 * To change this template use File | Settings | File Templates.
 */
public class StatQuery {
    public static final long ALL_PROVIDERS = -1;
    public static final long ALL_SERVICES = -1;

    private Date fromDate = new Date();
    private Date tillDate = new Date();

    private boolean fromDateEnabled = false;
    private boolean tillDateEnabled = false;

    private long providerId = ALL_PROVIDERS;
    private long serviceId = ALL_SERVICES;
    private long transport = 0;

    public StatQuery() {
        fromDate = Functions.truncateTime(fromDate);
    }

    public Date getFromDate() {
        return fromDate;
    }

    public void setFromDate(Date fromDate) {
        this.fromDate = fromDate;
    }

    public boolean isFromDateEnabled() {
        return fromDateEnabled;
    }

    public void setFromDateEnabled(boolean fromDateEnabled) {
        this.fromDateEnabled = fromDateEnabled;
    }

    public Date getTillDate() {
        return tillDate;
    }

    public void setTillDate(Date tillDate) {
        this.tillDate = tillDate;
    }

    public boolean isTillDateEnabled() {
        return tillDateEnabled;
    }

    public void setTillDateEnabled(boolean tillDateEnabled) {
        this.tillDateEnabled = tillDateEnabled;
    }

    public long getProviderId() {
        return providerId;
    }

    public void setProviderId(long providerId) {
        this.providerId = providerId;
    }

    public long getServiceId() {
        return serviceId;
    }

    public void setServiceId(long serviceId) {
        this.serviceId = serviceId;
    }

    public long getTransport() {
        return transport;
    }

    public void setTransport(long transport) {
        this.transport = transport;
    }
}
