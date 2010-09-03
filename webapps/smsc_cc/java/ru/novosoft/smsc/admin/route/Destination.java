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
}
