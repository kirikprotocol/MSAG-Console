package mobi.eyeline.informer.web.config;

import org.apache.myfaces.custom.aliasbean.AliasBean;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;

/**
 * @author Artem Snopkov
 */
public class InformerTimezones {

  private List<InformerTimezone> timezones;

  InformerTimezones(List<InformerTimezone> timezones) throws ConfigException {
    this.timezones = new ArrayList<InformerTimezone>(timezones.size());

    for (InformerTimezone tz : timezones) {
      if (getTimezoneByID(this.timezones, tz.getId()) != null)
        throw new ConfigException("Dublicate timezone: " + tz.getId());
      if (getTimezoneByAlias(this.timezones, tz.getAlias(new Locale("en")), new Locale("en")) != null)
        throw new ConfigException("Dublicate alias: " + tz.getAlias(new Locale("en")));
      if (getTimezoneByAlias(this.timezones, tz.getAlias(new Locale("ru")), new Locale("ru")) != null)
        throw new ConfigException("Dublicate alias: " + tz.getAlias(new Locale("ru")));
      this.timezones.add(tz);
    }

    this.timezones = Collections.unmodifiableList(this.timezones);
  }

  private InformerTimezone getTimezoneByID(List<InformerTimezone> zones, String id) {
    for (InformerTimezone tz : zones)
      if (tz.getId().equals(id))
        return tz;
    return null;
  }

  public InformerTimezone getTimezoneByID(String timezoneId) {
    return getTimezoneByID(timezones, timezoneId);
  }

  private InformerTimezone getTimezoneByAlias(List<InformerTimezone> zones, String alias, Locale locale) {
    for (InformerTimezone tz : zones) {
      if (tz.getAlias(locale).equals(alias))
        return tz;
    }
    return null;
  }

  public InformerTimezone getTimezoneByAlias(String alias, Locale locale) {
    return getTimezoneByAlias(timezones, alias, locale);
  }

  public List<InformerTimezone> getTimezones() {
    return timezones;
  }

}
