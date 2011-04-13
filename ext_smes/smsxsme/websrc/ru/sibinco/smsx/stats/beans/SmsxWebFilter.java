package ru.sibinco.smsx.stats.beans;

import ru.sibinco.smsx.stats.backend.datasource.WebDaily;
import ru.sibinco.smsx.stats.backend.datasource.WebRegion;

/**
 * @author Aleksandr Khalitov
 */
public class SmsxWebFilter {

  private String region;

  private Boolean msc;

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

  public boolean isAllowed(WebDaily webDaily) {
    if(region != null && region.length()>0 && !region.equals(webDaily.getRegion())) {
      return false;
    }
    if(msc != null && webDaily.isMsc() != msc.booleanValue()) {
      return false;
    }
    return true;
  }

  public boolean isAllowed(WebRegion webRegions) {
    if(region != null && region.length()>0 && !region.equals(webRegions.getRegion())) {
      return false;
    }
    if(msc != null && webRegions.isMsc() != msc.booleanValue()) {
      return false;
    }
    return true;
  }
}
