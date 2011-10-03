package ru.novosoft.smsc.admin.route;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;

/**
 * @author Artem Snopkov
 */
public class Source implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(Source.class);

  private Address mask;
  private String subject;

  Source(Source copy) {
    mask = copy.mask;
    subject = copy.subject;
  }

  public Source(Address mask) throws AdminException {
    vh.checkNotNull("mask", mask);
    this.mask = mask;
  }

  public Source(String subject) throws AdminException {
    vh.checkNotEmpty("subject", subject);
    this.subject = subject;
  }

  public Address getMask() {
    return mask;
  }

  public String getSubject() {
    return subject;
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    Source source = (Source) o;

    if (mask != null ? !mask.equals(source.mask) : source.mask != null) return false;
    if (subject != null ? !subject.equals(source.subject) : source.subject != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    int result = mask != null ? mask.hashCode() : 0;
    result = 31 * result + (subject != null ? subject.hashCode() : 0);
    return result;
  }

  @Override
  public String toString() {
    return "Source{" +
        "mask=" + mask +
        ", subject='" + subject + '\'' +
        '}';
  }
}
