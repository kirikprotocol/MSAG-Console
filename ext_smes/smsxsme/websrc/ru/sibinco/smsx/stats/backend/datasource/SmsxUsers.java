package ru.sibinco.smsx.stats.backend.datasource;

/**
* @author Aleksandr Khalitov
*/
public class SmsxUsers {
  private final int serviceId;
  private final String region;
  private final int count;

  public SmsxUsers(int serviceId, String region, int count) {
    this.serviceId = serviceId;
    this.region = region;
    this.count = count;
  }

  public int getServiceId() {
    return serviceId;
  }

  public String getRegion() {
    return region;
  }

  public int getCount() {
    return count;
  }

  public boolean equals(Object o) {
    if (this == o) return true;
    if (!(o instanceof SmsxUsers)) return false;

    SmsxUsers smsxUsers = (SmsxUsers) o;

    if (count != smsxUsers.count) return false;
    if (serviceId != smsxUsers.serviceId) return false;
    if (region != null ? !region.equals(smsxUsers.region) : smsxUsers.region != null) return false;

    return true;
  }

  public int hashCode() {
    int result = serviceId;
    result = 31 * result + (region != null ? region.hashCode() : 0);
    result = 31 * result + count;
    return result;
  }
}
