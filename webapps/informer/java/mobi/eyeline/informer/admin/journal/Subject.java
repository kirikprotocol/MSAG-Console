package mobi.eyeline.informer.admin.journal;

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.ResourceBundle;

/**
 * Сабжект записи в журнале
 *
 * @author Aleksandr Khalitov
 */
public enum Subject {

  USERS("subject.user"),
  CONFIG("subject.config"),
  BLACKLIST("subject.blacklist"),
  SMSC("subject.smsc"),
  REGIONS("subject.regions"),
  DAEMONS("subject.daemons"),
  INFORMER("subject.informer"),
  FTPSERVER("subject.ftpserver"),
  ARCHIVE_DAEMON("subject.archivedaemon"),
  DELIVERIES("subject.deliveries"),
  RESTRICTIONS("subject.restrictions"),
  WEBCONFIG("subject.webconfig"),
  PVSS("subject.pvss");

  private static final Map<String, Subject> map = new HashMap<String, Subject>(values().length) {
    {
      for (Subject s : Subject.values()) {
        put(s.key, s);
      }
    }
  };
  private final String key;

  Subject(String key) {
    this.key = key;
  }

  /**
   * Возвращает ключ названия в ResourceBundle-e
   *
   * @return ключ названия в ResourceBundle-e
   */
  public String getKey() {
    return key;
  }

  /**
   * Возвращает название сабжекта в указанной локали
   *
   * @param locale локаль
   * @return название сабжекта в указанной локали
   */
  public String getSubject(Locale locale) {
    ResourceBundle rb = ResourceBundle.getBundle(JournalRecord.class.getCanonicalName(), locale);
    return rb.getString(key);
  }

  /**
   * Возвращает сабжект по ключу в ResourceBundle-e
   *
   * @param key ключ в ResourceBundle-e
   * @return сабжект
   */
  public static Subject getByKey(String key) {
    if (key == null) {
      return null;
    }
    return map.get(key);
  }
}