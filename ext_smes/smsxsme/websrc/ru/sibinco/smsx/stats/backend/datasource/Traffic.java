package ru.sibinco.smsx.stats.backend.datasource;


/**
 * @author Aleksandr Khalitov
 */
public class Traffic {

  private final String region;
  private final int serviceId;
  private final boolean msc;
  private final int count;

  public Traffic(String region, int serviceId, boolean msc, int count) {//todo уменьшить видимость конструктора
    this.region = region;
    this.serviceId = serviceId;
    this.msc = msc;
    this.count = count;
  }

  public String getRegion() {
    return region;
  }

  public int getServiceId() {
    return serviceId;
  }

  public boolean isMsc() {
    return msc;
  }

  public int getCount() {
    return count;
  }

  public boolean equals(Object o) {
    if (this == o) return true;
    if (!(o instanceof Traffic)) return false;

    Traffic traffic = (Traffic) o;

    if (count != traffic.count) return false;
    if (msc != traffic.msc) return false;
    if (serviceId != traffic.serviceId) return false;
    if (region != null ? !region.equals(traffic.region) : traffic.region != null) return false;

    return true;
  }

  public int hashCode() {
    int result = region != null ? region.hashCode() : 0;
    result = 31 * result + serviceId;
    result = 31 * result + (msc ? 1 : 0);
    result = 31 * result + count;
    return result;
  }
}
