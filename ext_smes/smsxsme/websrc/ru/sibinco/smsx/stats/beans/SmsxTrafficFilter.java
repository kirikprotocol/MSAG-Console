package ru.sibinco.smsx.stats.beans;

import ru.sibinco.smsx.stats.backend.datasource.Traffic;

/**
 * @author Aleksandr Khalitov
 */
class SmsxTrafficFilter {

  private Integer serviceId;

  private String region;

  private Boolean msc;

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

  public Boolean getMsc() {
    return msc;
  }

  public void setMsc(Boolean msc) {
    this.msc = msc;
  }

  public boolean isAllowed(Traffic traffic) {
    if(serviceId != null && traffic.getServiceId() != serviceId.intValue()) {
      return false;
    }
    if(region != null && region.length()>0 && !region.equals(traffic.getRegion())) {
      return false;
    }
    if(msc != null && traffic.isMsc() != msc.booleanValue()) {
      return false;
    }
    return true;
  }
}
