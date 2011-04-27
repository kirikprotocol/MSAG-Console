package ru.sibinco.sponsored.stats.beans;

import ru.sibinco.sponsored.stats.backend.SponsoredRecord;

/**
 * @author Aleksandr Khalitov
 */
class SponsoredResultFilter {

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
