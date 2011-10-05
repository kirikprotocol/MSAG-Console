package ru.novosoft.smsc.admin.stat;

/**
 * author: Aleksandr Khalitov
 */
public class RouteIdCountersSet extends ExtendedCountersSet implements Comparable {

  private String routeid;
  private long providerId = -1;
  private long categoryId = -1;

  public RouteIdCountersSet(String routeid) {
    this.routeid = routeid;
  }

  public RouteIdCountersSet(long accepted, long rejected, long delivered,
                            long failed, long rescheduled, long temporal, long i, long o, String routeid) {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.routeid = routeid;
  }

  public RouteIdCountersSet(long accepted, long rejected, long delivered,
                            long failed, long rescheduled, long temporal, long i, long o, String routeid,
                            long providerId, long categoryId) {
    super(accepted, rejected, delivered, failed, rescheduled, temporal, i, o);
    this.routeid = routeid;
    this.providerId = providerId;
    this.categoryId = categoryId;
  }

  public void setCategoryId(long categoryId) {
    this.categoryId = categoryId;
  }

  public void setProviderId(long providerId) {
    this.providerId = providerId;
  }

  public int compareTo(Object o) {
    if (o == null || routeid == null || !(o instanceof RouteIdCountersSet)) return -1;
    return routeid.compareTo(((RouteIdCountersSet) o).routeid);
  }

  public String getRouteid() {
    return routeid;
  }

  public long getProviderId() {
    return providerId;
  }

  public long getCategoryId() {
    return categoryId;
  }
}
