package ru.novosoft.smsc.web.config.timezone;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.config.SmscConfigurationStatus;
import ru.novosoft.smsc.admin.timezone.Timezone;
import ru.novosoft.smsc.admin.timezone.TimezoneManager;
import ru.novosoft.smsc.admin.timezone.TimezoneSettings;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.web.config.BaseSettingsManager;
import ru.novosoft.smsc.web.journal.Journal;

import java.util.Collection;
import java.util.List;
import java.util.Map;

/**
 * @author Artem Snopkov
 */
public class WTimezoneManager extends BaseSettingsManager<TimezoneSettings> implements TimezoneManager {

  private final TimezoneManager wrapped;
  private final Journal j;

  public WTimezoneManager(TimezoneManager wrapped, Journal j, String user) {
    super(user);
    this.wrapped = wrapped;
    this.j = j;
  }

  @Override
  protected void _updateSettings(TimezoneSettings settings) throws AdminException {
    TimezoneSettings oldSettings = getSettings();
    wrapped.updateSettings(settings);


  }

  public TimezoneSettings getSettings() throws AdminException {
    return wrapped.getSettings();
  }

  public TimezoneSettings cloneSettings(TimezoneSettings settings) {
    return settings.cloneSettings();
  }

  public Map<Integer, SmscConfigurationStatus> getStatusForSmscs() throws AdminException {
    return wrapped.getStatusForSmscs();
  }

  private static Timezone getTimezoneBySubject(Collection<Timezone> tzones, String subject) {
    for (Timezone tz : tzones)
      if (tz.getSubject() != null && tz.getSubject().equals(subject))
        return tz;
    return null;
  }

  private static Timezone getTimezoneByMask(Collection<Timezone> tzones, Address mask) {
    for (Timezone tz : tzones)
      if (tz.getMask() != null && tz.getMask().equals(mask))
        return tz;
    return null;
  }

  protected void logChanges(TimezoneSettings oldSettings, TimezoneSettings newSettings) {
    if (!oldSettings.getDefaultTimeZone().equals(newSettings.getDefaultTimeZone()))
      j.user(user).change("default_timezone_changed", oldSettings.getDefaultTimeZone().getDisplayName(), newSettings.getDefaultTimeZone().getDisplayName()).timezone();

    Collection<Timezone> oldTimezones = oldSettings.getTimezones();
    Collection<Timezone> newTimezones = newSettings.getTimezones();

    for (Timezone otz : oldTimezones) {
      Timezone ntz;
      String otzId;
      if (otz.getMask() != null) {
        ntz = getTimezoneByMask(newTimezones, otz.getMask());
        otzId = otz.getMask().getSimpleAddress();
      } else {
        ntz = getTimezoneBySubject(newTimezones, otz.getSubject());
        otzId = otz.getSubject();
      }

      if (ntz == null)
        j.user(user).remove().timezone(otzId);
      else if (!otz.getTimezone().equals(ntz.getTimezone()))
        j.user(user).change("timezone_changed", otz.getTimezone().getDisplayName(), ntz.getTimezone().getDisplayName()).timezone(otzId);
    }

    for (Timezone ntz : newTimezones) {
      Timezone otz;
      String ntzId;
      if (ntz.getMask() != null) {
        otz = getTimezoneByMask(oldTimezones, ntz.getMask());
        ntzId = ntz.getMask().getSimpleAddress();
      } else {
        otz = getTimezoneBySubject(oldTimezones, ntz.getSubject());
        ntzId = ntz.getSubject();
      }

      if (otz == null)
        j.user(user).add().timezone(ntzId);
    }
  }
}
