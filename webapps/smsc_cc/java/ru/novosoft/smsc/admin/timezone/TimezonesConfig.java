package ru.novosoft.smsc.admin.timezone;

import ru.novosoft.smsc.admin.config.ManagedConfigFile;
import ru.novosoft.smsc.util.Address;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigParam;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.TimeZone;

/**
 * @author Artem Snopkov
 */
public class TimezonesConfig implements ManagedConfigFile<TimezoneSettings> {
  
  public void save(InputStream oldFile, OutputStream newFile, TimezoneSettings conf) throws Exception {
    XmlConfig config = new XmlConfig(oldFile);
    if (config.containsSection("masks"))
      config.removeSection("masks");
    if (config.containsSection("subjects"))
      config.removeSection("subjects");

    XmlConfigSection masks = config.addSection("masks");
    XmlConfigSection subjects = config.addSection("subjects");

    for (Timezone tz : conf.getTimezones()) {
      if (tz.getMask() != null)
        masks.setString(tz.getMask().getSimpleAddress(), formatTimezone(tz.getTimezone()));
      else
        subjects.setString(tz.getSubject(), formatTimezone(tz.getTimezone()));
    }

    config.setString("default_timezone", formatTimezone(conf.getDefaultTimeZone()));

    config.save(newFile);
  }

  public TimezoneSettings load(InputStream is) throws Exception {
    XmlConfig config = new XmlConfig(is);

    TimeZone defaultTimeZone = parseTimezone(config.getString("default_timezone"));
    Collection<Timezone> timezones = new ArrayList<Timezone>();

    if (config.containsSection("masks")) {
      for (XmlConfigParam p : config.getSection("masks").params())
        timezones.add(new Timezone(new Address(p.getName()),parseTimezone(p.getString())));
    }

    if (config.containsSection("subjects")) {
      for (XmlConfigParam p : config.getSection("subjects").params())
        timezones.add(new Timezone(p.getName(), parseTimezone(p.getString())));
    }

    return new TimezoneSettings(defaultTimeZone, timezones);
  }

  private static String formatTimezone(TimeZone timezone) {
    return timezone.getRawOffset() / 60000 + "," + timezone.getID();
  }

  private static TimeZone parseTimezone(String value) {
    int i = value.indexOf(',');
    if (i  > 0)
      value = value.substring(i+1);
    return TimeZone.getTimeZone(value);
  }
}
