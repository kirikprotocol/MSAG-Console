package mobi.eyeline.informer.web.controllers.delivery;

import mobi.eyeline.informer.util.Address;

import java.io.Serializable;
import java.util.Date;

/**
 * @author Aleksandr Khalitov
 */
public class MsgFilter implements Serializable {

  private Address msisdn;

  private MsgState state;

  private Date fromDate;

  private Date tillDate;

  private Integer errorCode;

  public MsgFilter() {
  }

  public MsgFilter(MsgFilter filter) {
    msisdn = filter.msisdn == null ? null : new Address(filter.msisdn);
    state = filter.state;
    fromDate = filter.fromDate == null ? null : new Date(fromDate.getTime());
    tillDate = filter.tillDate == null ? null : new Date(tillDate.getTime());
    errorCode = filter.errorCode;
  }

  public Address getMsisdn() {
    return msisdn;
  }

  public void setMsisdn(Address msisdn) {
    this.msisdn = msisdn;
  }

  public MsgState getState() {
    return state;
  }

  public void setState(MsgState state) {
    this.state = state;
  }

  public Date getFromDate() {
    return fromDate;
  }

  public void setFromDate(Date fromDate) {
    this.fromDate = fromDate;
  }

  public Date getTillDate() {
    return tillDate;
  }

  public void setTillDate(Date tillDate) {
    this.tillDate = tillDate;
  }

  public Integer getErrorCode() {
    return errorCode;
  }

  public void setErrorCode(Integer errorCode) {
    this.errorCode = errorCode;
  }
}
