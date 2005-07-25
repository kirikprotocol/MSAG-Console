package ru.sibinco.scag.backend.stat.stat;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 26.08.2004
 * Time: 13:51:48
 * To change this template use File | Settings | File Templates.
 */
public class CountersSet
{
  protected long accepted = 0;
  protected long rejected = 0;
  protected long delivered = 0;
  protected long gw_rejected = 0;
  protected long failed = 0;
  protected long billingOk = 0;
  protected long billingFailed = 0;
  protected long recieptOk = 0;
  protected long recieptFailed = 0; 

  public CountersSet() {}

  public CountersSet(long accepted, long rejected, long delivered,
                     long gw_rejected, long failed)
  {
    this.accepted = accepted;  this.rejected = rejected; this.delivered = delivered;
    this.gw_rejected = gw_rejected; this.failed = failed;
  }

  public CountersSet(long accepted, long rejected, long delivered,
                     long gw_rejected, long failed, long billingOk,
                     long billingFailed, long recieptOk, long recieptFailed)
  {
    this.accepted = accepted;  this.rejected = rejected; this.delivered = delivered;
    this.gw_rejected = gw_rejected; this.failed = failed; this.billingOk = billingOk;
    this.billingFailed = billingFailed; this.recieptOk = recieptOk; this.recieptFailed = recieptFailed;
  }

 protected void increment(long accepted, long rejected, long delivered,
                     long gw_rejected, long failed)
    {
        this.accepted += accepted; this.rejected += rejected;
        this.delivered += delivered; this.gw_rejected += gw_rejected;
        this.failed += failed;
    }

  protected void incrementFull(long accepted, long rejected, long delivered,
                     long gw_rejected, long failed, long billingOk,
                     long billingFailed, long recieptOk, long recieptFailed)
    {
        this.accepted += accepted; this.rejected += rejected;
        this.delivered += delivered; this.gw_rejected += gw_rejected;
        this.failed += failed; this.billingOk += billingOk; this.billingFailed += billingFailed;
        this.recieptOk += recieptOk; this.recieptFailed += recieptFailed;
    }

  protected void incrementFull(CountersSet set)
  {
      this.accepted += set.accepted; this.rejected += set.rejected;
      this.delivered += set.delivered; this.gw_rejected += set.gw_rejected;
      this.failed += set.failed; this.billingOk += set.billingOk; this.billingFailed += set.billingFailed;
      this.recieptOk += set.recieptOk; this.recieptFailed += set.recieptFailed;
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
}
