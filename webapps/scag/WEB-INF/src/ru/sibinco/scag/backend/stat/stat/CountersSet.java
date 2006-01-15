package ru.sibinco.scag.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:51:48
 * To change this template use File | Settings | File Templates.
 */
public class CountersSet {

    protected long accepted = 0;
    protected long rejected = 0;
    protected long delivered = 0;
    protected long gw_rejected = 0;
    protected long failed = 0;
    protected long billingOk = 0;
    protected long billingFailed = 0;
    protected long recieptOk = 0;
    protected long recieptFailed = 0;

    protected long request = 0;
    protected long requestRejected = 0;
    protected long response = 0;
    protected long responseRejected = 0;


    public CountersSet() {
    }

    /**
     * Constructor for HTTP Route records
     * @param request - request parameter
     * @param requestRejected - requestRejected parameter
     * @param response - response parameter
     * @param responseRejected - responseRejected parameter
     * @param delivered - delivered parameter
     * @param failed  - failed parameter
     * @param billingOk  - billingOk parameter
     * @param billingFailed - billingFailed parameter
     */
    public CountersSet(long request, long requestRejected, long response,
                       long responseRejected, long delivered, long failed,
                       long billingOk, long billingFailed) {
        this.request = request;
        this.requestRejected = requestRejected;
        this.response = response;
        this.responseRejected = responseRejected;
        this.delivered = delivered;
        this.failed = failed;
        this.billingOk = billingOk;
        this.billingFailed = billingFailed;
    }

    /**
     * Constructor for HTTP Service records
     * @param request - request parameter
     * @param requestRejected - requestRejected parameter
     * @param response - response parameter
     * @param responseRejected - responseRejected parameter
     * @param delivered - delivered parameter
     * @param failed  - failed parameter
     */
    public CountersSet(long request, long requestRejected, long response,
                       long responseRejected, long delivered, long failed) {
        this.request = request;
        this.requestRejected = requestRejected;
        this.response = response;
        this.responseRejected = responseRejected;
        this.delivered = delivered;
        this.failed = failed;
    }

    public CountersSet(long accepted, long rejected, long delivered,
                       long gw_rejected, long failed) {
        this.accepted = accepted;
        this.rejected = rejected;
        this.delivered = delivered;
        this.gw_rejected = gw_rejected;
        this.failed = failed;
    }

    public CountersSet(long accepted, long rejected, long delivered,
                       long gw_rejected, long failed, long billingOk,
                       long billingFailed, long recieptOk, long recieptFailed) {
        this.accepted = accepted;
        this.rejected = rejected;
        this.delivered = delivered;
        this.gw_rejected = gw_rejected;
        this.failed = failed;
        this.billingOk = billingOk;
        this.billingFailed = billingFailed;
        this.recieptOk = recieptOk;
        this.recieptFailed = recieptFailed;
    }

    protected void incrementForSMPPTransport(long accepted, long rejected, long delivered,
                                             long gw_rejected, long failed) {
        this.accepted += accepted;
        this.rejected += rejected;
        this.delivered += delivered;
        this.gw_rejected += gw_rejected;
        this.failed += failed;
    }

    protected void incrementForHttpTransport(long request, long requestRejected, long response,
                       long responseRejected, long delivered, long failed) {
        this.request += request;
        this.requestRejected += requestRejected;
        this.response += response;
        this.responseRejected += responseRejected;
        this.delivered += delivered;
        this.failed += failed;
    }

    protected void incrementFullForSMPPTransport(long accepted, long rejected, long delivered,
                                                 long gw_rejected, long failed, long billingOk,
                                                 long billingFailed, long recieptOk, long recieptFailed) {
        this.accepted += accepted;
        this.rejected += rejected;
        this.delivered += delivered;
        this.gw_rejected += gw_rejected;
        this.failed += failed;
        this.billingOk += billingOk;
        this.billingFailed += billingFailed;
        this.recieptOk += recieptOk;
        this.recieptFailed += recieptFailed;
    }

    protected void incrementFullForHttpTransport(long request, long requestRejected, long response,
                       long responseRejected, long delivered, long failed,
                       long billingOk, long billingFailed) {
        this.request += request;
        this.requestRejected += requestRejected;
        this.response += response;
        this.responseRejected += responseRejected;
        this.delivered += delivered;
        this.failed += failed;
        this.billingOk += billingOk;
        this.billingFailed += billingFailed;
    }

    protected void incrementFullForSMPPTransport(CountersSet set) {
        this.accepted += set.accepted;
        this.rejected += set.rejected;
        this.delivered += set.delivered;
        this.gw_rejected += set.gw_rejected;
        this.failed += set.failed;
        this.billingOk += set.billingOk;
        this.billingFailed += set.billingFailed;
        this.recieptOk += set.recieptOk;
        this.recieptFailed += set.recieptFailed;
    }

    protected void incrementFullForHttpTransport(CountersSet set) {
        this.request += set.request;
        this.requestRejected += set.requestRejected;
        this.response += set.response;
        this.responseRejected += set.responseRejected;
        this.delivered += set.delivered;
        this.failed += set.failed;
        this.billingOk += set.billingOk;
        this.billingFailed += set.billingFailed;
    }

    public long getAccepted() {
        return accepted;
    }

    public long getRejected() {
        return rejected;
    }

    public long getDelivered() {
        return delivered;
    }

    public long getGw_rejected() {
        return gw_rejected;
    }

    public long getFailed() {
        return failed;
    }

    public long getBillingOk() {
        return billingOk;
    }

    public long getBillingFailed() {
        return billingFailed;
    }

    public long getRecieptOk() {
        return recieptOk;
    }

    public long getRecieptFailed() {
        return recieptFailed;
    }

    public long getRequest() {
        return request;
    }

    public long getRequestRejected() {
        return requestRejected;
    }

    public long getResponse() {
        return response;
    }

    public long getResponseRejected() {
        return responseRejected;
    }
}
