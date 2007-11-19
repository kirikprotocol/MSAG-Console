package ru.sibinco.smpp.ub_sme.outgoing;

import java.util.Comparator;

public class RetryTask implements Comparator {

    private OutgoingObject outgoingObject;
    private long retryTime = -1;
    private int oosn = -1;

    public RetryTask() {
    }

    public RetryTask(OutgoingObject outgoingObject) {
        this.outgoingObject = outgoingObject;
        this.retryTime = System.currentTimeMillis() + outgoingObject.getRetryPeriod();
        this.oosn = outgoingObject.getOutgoingMessage().getSequenceNumber();
    }

    public OutgoingObject getOutgoingObject() {
        return outgoingObject;
    }

    public long getRetryTime() {
        return retryTime;
    }

    public long getSequenceNumber() {
        return oosn;
    }

    public int getRetries() {
        return outgoingObject.getRetries();
    }

    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof RetryTask)) return false;

        final RetryTask retryTask = (RetryTask) o;

        if (retryTime != retryTask.retryTime) return false;
        if (oosn != retryTask.oosn) return false;
        return true;
    }

    public int compare(Object obj1, Object obj2) {
        if (obj1 instanceof RetryTask && obj2 instanceof RetryTask) {
            int ret = (int) (((RetryTask) obj1).getRetryTime() - ((RetryTask) obj2).getRetryTime());
            if (ret == 0) ret = (int) (((RetryTask) obj1).getSequenceNumber() - ((RetryTask) obj2).getSequenceNumber());
            return ret;
        } else {
            return 0;
        }
    }
}
