package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;

/**
 * @author Artem Snopkov
 */
public class Destination implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(Destination.class);

  private Address mask;
  private String subject;
  private String smeId;

  public Destination(Address mask, String smeId) throws AdminException {
    vh.checkNotNull("mask", mask);
    vh.checkNotEmpty("smeId", smeId);
    this.mask = mask;
    this.smeId = smeId;
  }

  public Destination(String subject, String smeId) throws AdminException {
    vh.checkNotEmpty("subject", subject);
    vh.checkNotEmpty("smeId", smeId);
    this.subject = subject;
    this.smeId = smeId;
  }

  public Address getMask() {
    return mask;
  }

  public String getSubject() {
    return subject;
  }

  public String getSmeId() {
    return smeId;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Destination that = (Destination) o;

    if (mask != null ? !mask.equals(that.mask) : that.mask != null) return false;
    if (smeId != null ? !smeId.equals(that.smeId) : that.smeId != null) return false;
    if (subject != null ? !subject.equals(that.subject) : that.subject != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = mask != null ? mask.hashCode() : 0;
    result = 31 * result + (subject != null ? subject.hashCode() : 0);
    result = 31 * result + (smeId != null ? smeId.hashCode() : 0);
    return result;
  }

  @Override
  public String toString() {
    return "Destination{" +
        "mask=" + mask +
        ", subject='" + subject + '\'' +
        ", smeId='" + smeId + '\'' +
        '}';
  }
}
