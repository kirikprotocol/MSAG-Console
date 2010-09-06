package ru.novosoft.smsc.admin.timezone;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.util.ValidationHelper;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.TimeZone;

/**
 * @author Artem Snopkov
 */
public class TimezoneSettings implements Serializable {

  private static final transient ValidationHelper vh = new ValidationHelper(TimezoneSettings.class);

  private TimeZone defaultTimeZone;
  private Collection<Timezone> timezones;

  TimezoneSettings(TimeZone defaultTimeZone, Collection<Timezone> timezones) {
    this.defaultTimeZone = defaultTimeZone;
    this.timezones = timezones;
  }

  TimezoneSettings(TimezoneSettings copy) {
    this.defaultTimeZone = copy.defaultTimeZone;
    this.timezones = new ArrayList<Timezone>(copy.timezones);
  }

  public TimeZone getDefaultTimeZone() {
    return defaultTimeZone;
  }

  public void setDefaultTimeZone(TimeZone defaultTimeZone) throws AdminException {
    vh.checkNotNull("defaultTimezone", defaultTimeZone);
    this.defaultTimeZone = defaultTimeZone;
  }

  public void setTimezones(Collection<Timezone> timezones) throws AdminException {
    vh.checkNoNulls("timezones", timezones);
    this.timezones = new ArrayList<Timezone>(timezones);
  }

  public Collection<Timezone> getTimezones() {
    return new ArrayList<Timezone>(timezones);
  }

  public TimezoneSettings cloneSettings() {
    return new TimezoneSettings(this);
  }

}
