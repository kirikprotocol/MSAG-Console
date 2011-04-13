package ru.sibinco.sponsored.stats.beans;

import ru.sibinco.sponsored.stats.backend.datasource.SponsoredRecord;

/**
 * @author Aleksandr Khalitov
 */
public class SponsoredResultFilter {

  private String addressPrefix;


  public String getAddressPrefix() {
    return addressPrefix;
  }

  public void setAddressPrefix(String addressPrefix) {
    this.addressPrefix = addressPrefix;
  }

  public boolean isAllowed(SponsoredRecord r) {
    if(addressPrefix != null && !r.getAddress().startsWith(addressPrefix)) {
      return false;
    }
    return true;
  }
}
