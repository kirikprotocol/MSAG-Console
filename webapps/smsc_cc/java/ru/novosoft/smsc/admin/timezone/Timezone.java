package ru.novosoft.smsc.admin.timezone;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;
import ru.novosoft.smsc.util.Address;

import java.io.Serializable;
import java.util.TimeZone;

/**
 * @author Artem Snopkov
 */
public class Timezone implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(Timezone.class);

  private final TimeZone timezone;
  private final Address mask;
  private final String subject;

  public Timezone(Address mask, TimeZone timezone) throws AdminException {
    vh.checkNotNull("timezone", timezone);
    this.timezone = timezone;
    this.mask = mask;
    this.subject = null;
  }

  public Timezone(String subject, TimeZone timezone) throws AdminException {
    vh.checkNotNull("subject", subject);
    vh.checkNotNull("timezone", timezone);
    this.timezone = timezone;
    this.subject = subject;
    this.mask = null;
  }

  public TimeZone getTimezone() {
    return timezone;
  }

  public Address getMask() {
    return mask;
  }

  public String getSubject() {
    return subject;
  }
}
