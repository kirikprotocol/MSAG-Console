package mobi.eyeline.informer.admin.retry_policies;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.10.2010
 * Time: 14:13:57
 */
public class RetryPolicyEntry {

  Integer errCode;
  Integer time;
  private ValidationHelper vh = new ValidationHelper(RetryPolicySettings.class);

  public RetryPolicyEntry(Integer errCode, Integer time) throws AdminException {
    vh.checkPositive("errorCode",errCode);
    vh.checkPositive("time",time);
    this.errCode = errCode;
    this.time = time;
  }

  public RetryPolicyEntry(RetryPolicyEntry copy) {
    this.errCode = copy.errCode;
    this.time    = copy.time;
  }

  public Integer getErrCode() {
    return errCode;
  }

  public void setErrCode(Integer errCode) throws AdminException {
    vh.checkPositive("errorCode",errCode);
    this.errCode = errCode;
  }

  public Integer getTime() {
    return time;
  }

  public void setTime(Integer time) throws AdminException {
    vh.checkPositive("time",time);
    this.time = time;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    RetryPolicyEntry that = (RetryPolicyEntry) o;

    if (errCode != null ? !errCode.equals(that.errCode) : that.errCode != null) return false;
    if (time != null ? !time.equals(that.time) : that.time != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = errCode != null ? errCode.hashCode() : 0;
    result = 31 * result + (time != null ? time.hashCode() : 0);
    return result;
  }

  @Override
  public String toString() {
    return "Entry{" +
        "errCode=" + errCode +
        ", time=" + time +
        '}';
  }
}
