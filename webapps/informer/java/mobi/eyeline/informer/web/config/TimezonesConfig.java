package mobi.eyeline.informer.web.config;

import mobi.eyeline.informer.util.config.XmlConfig;
import mobi.eyeline.informer.util.config.XmlConfigException;
import mobi.eyeline.informer.util.config.XmlConfigParam;
import mobi.eyeline.informer.util.config.XmlConfigSection;

import java.io.File;
import java.util.*;

/**
 * @author Artem Snopkov
 */
public class TimezonesConfig {

  public File file;

  public TimezonesConfig(File file) {
    this.file = file;
  }

  /**
   * Возвращает список часовых поясов из файла в том же порядке, в котором они находятся в файле
   * @return список часовых поясов из файла в том же порядке, в котором они находятся в файле
   * @throws ConfigException если произошла ошибка при чтении конфигурационного файла
   */
  public InformerTimezones getTimezones() throws ConfigException {
    XmlConfig cfg = new XmlConfig();
    try {
      cfg.load(file);
    } catch (XmlConfigException e) {
      throw new ConfigException(e);
    }

    List<InformerTimezone> result = new ArrayList<InformerTimezone>(cfg.getSectionsCount());

    for (XmlConfigSection t : cfg.sections())
      result.add(deserializeTimezone(t));

    return new InformerTimezones(result);
  }

  private static boolean isTimezoneExists(String name) {
    for (String id : TimeZone.getAvailableIDs())
      if (id.equals(name))
        return true;
    return false;
  }

  private static InformerTimezone deserializeTimezone(XmlConfigSection s) throws ConfigException {
    if (!isTimezoneExists(s.getName()))
      throw new ConfigException("Timezone does not exists: " + s.getName());

    TimeZone tz = TimeZone.getTimeZone(s.getName());
    Map<String, String> aliases = null;
    if (s.containsSection("aliases")) {
      aliases = new HashMap<String, String>();
      try {
        XmlConfigSection aliasesSec = s.getSection("aliases");
        for (XmlConfigParam p : aliasesSec.params())
          aliases.put(p.getName(), p.getString());

        if (!aliases.containsKey("en"))
          throw new ConfigException("No alias found for tz: " + s.getName() + " and locale: en");

        if (!aliases.containsKey("ru"))
          throw new ConfigException("No alias found for tz: " + s.getName() + " and locale: ru");

      } catch (XmlConfigException e) {
        throw new ConfigException(e);
      }
    }

    return new InformerTimezone(tz, aliases);
  }

}
