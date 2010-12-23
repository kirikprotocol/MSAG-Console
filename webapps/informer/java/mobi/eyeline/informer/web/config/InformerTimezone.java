package mobi.eyeline.informer.web.config;

import java.util.Collections;
import java.util.Locale;
import java.util.Map;
import java.util.TimeZone;

/**
 * @author Artem Snopkov
 */
public class InformerTimezone {

  private TimeZone timezone;
  private Map<String, String> aliases;

  InformerTimezone(TimeZone timezone, Map<String, String> aliases) {
    this.timezone = timezone;
    if (aliases == null)
      this.aliases = Collections.emptyMap();
    else
      this.aliases = Collections.unmodifiableMap(aliases);
  }

  public TimeZone getTimezone() {
    return timezone;
  }

  public String getId() {
    return timezone.getID();
  }

  public Map<String, String> getAliases() {
    return aliases;
  }

  public String getAlias(Locale locale) {
    if (locale == null)
      return null;
    return aliases.get(locale.getLanguage());
  }

  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    InformerTimezone that = (InformerTimezone) o;

    if (timezone != null ? !timezone.equals(that.timezone) : that.timezone != null) return false;

    return true;
  }

  @Override
  public int hashCode() {
    return timezone != null ? timezone.hashCode() : 0;
  }
}
