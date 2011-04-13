package ru.sibinco.smsx.stats.beans;

import ru.sibinco.smsx.stats.backend.datasource.SmsxUsers;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxUsersFilter{

  private Integer serviceId;

  private String region;

  public Integer getServiceId() {
    return serviceId;
  }

  public void setServiceId(Integer serviceId) {
    this.serviceId = serviceId;
  }

  public String getRegion() {
    return region;
  }

  public void setRegion(String region) {
    this.region = region;
  }

  public boolean isAllowed(SmsxUsers users) {
    if(serviceId != null && users.getServiceId() != serviceId.intValue()) {
      return false;
    }
    if(region != null && region.length()>0 && !region.equals(users.getRegion())) {
      return false;
    }
    return true;
  }


}
