package ru.sibinco.sponsored.stats.beans;

import ru.sibinco.sponsored.stats.backend.SponsoredRequest;

/**
 * @author Aleksandr Khalitov
 */
class RequestFilter {

  private SponsoredRequest.Status status;


  public SponsoredRequest.Status getStatus() {
    return status;
  }

  public void setStatus(SponsoredRequest.Status status) {
    this.status = status;
  }


  public boolean isAllowed(SponsoredRequest request) {
    return !(status != null && request.getStatus() != status);
  }
}
