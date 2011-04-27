package ru.sibinco.smsx.stats.beans;

import ru.sibinco.smsx.stats.backend.SmsxRequest;

/**
 * @author Aleksandr Khalitov
 */
class RequestFilter {

  private SmsxRequest.Status status;


  public SmsxRequest.Status getStatus() {
    return status;
  }

  public void setStatus(SmsxRequest.Status status) {
    this.status = status;
  }


  public boolean isAllowed(SmsxRequest request) {
    return !(status != null && request.getStatus() != status);
  }
}
