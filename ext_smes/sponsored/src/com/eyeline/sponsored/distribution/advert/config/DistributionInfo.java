package com.eyeline.sponsored.distribution.advert.config;

import java.util.Calendar;

/**
 * User: artem
 * Date: 01.02.2008
 */

public class DistributionInfo {
  private String distributionName;
  private Calendar startTime;
  private Calendar endTime;
  private String srcAddress;
  private String advServiceName;

  public String getDistributionName() {
    return distributionName;
  }

  public void setDistributionName(String distributionName) {
    this.distributionName = distributionName;
  }

  public Calendar getStartTime() {
    return startTime;
  }

  public void setStartTime(Calendar startTime) {
    this.startTime = startTime;
  }

  public Calendar getEndTime() {
    return endTime;
  }

  public void setEndTime(Calendar endTime) {
    this.endTime = endTime;
  }

  public String getSrcAddress() {
    return srcAddress;
  }

  public void setSrcAddress(String srcAddress) {
    this.srcAddress = srcAddress;
  }

  public String getAdvServiceName() {
    return advServiceName;
  }

  public void setAdvServiceName(String advServiceName) {
    this.advServiceName = advServiceName;
  }
}
