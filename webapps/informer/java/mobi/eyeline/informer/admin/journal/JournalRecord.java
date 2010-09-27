package mobi.eyeline.informer.admin.journal;

import java.util.Arrays;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * Структура, описывающая одну запись в журнале
 * @author Artem Snopkov
 */
public class JournalRecord {

  private long time;
  private Subject subject;
  private String user;
  private final Type type;
  private String descriptionKey;
  private String[] descriptionArgs;

  JournalRecord(Type type) {
    this.type = type;
  }

  /**
   * Возвращает время в миллисекундах, в которое произошло изменение
   * @return время в миллисекундах, в которое произошло изменение
   */
  public long getTime() {
    return time;
  }

  JournalRecord setTime(long time) {
    this.time = time;
    return this;
  }

  /**
   * Возвращает название сабжекта
   * @return название сабжекта в указанной локали
   */
  public Subject getSubject() {
    return subject;
  }

  JournalRecord setSubject(Subject subject) {
    this.subject = subject;
    return this;
  }

  /**
   * Возвращает логин пользователь, произвевшего изменение
   * @return логин пользователь, произвевшего изменение
   */
  public String getUser() {
    return user;
  }

  JournalRecord setUser(String user) {
    this.user = user;
    return this;
  }

  JournalRecord setDescription(String key, String... args) {
    this.descriptionKey = key;
    this.descriptionArgs = args;
    return this;
  }

  String getDescriptionKey() {
    return descriptionKey;
  }

  String[] getDescriptionArgs() {
    return descriptionArgs;
  }

  /**
   * Возвращает описание изменения в заданной локали
   * @param locale локаль
   * @return описание изменения в заданной локали
   */
  public String getDescription(Locale locale) {
    String str = ResourceBundle.getBundle(JournalRecord.class.getCanonicalName(), locale).getString(descriptionKey);
    if (descriptionArgs != null) {
      for (int i=0; i<descriptionArgs.length; i++)
        str = str.replaceAll("\\{" + i + "\\}", descriptionArgs[i]);
    }
    return str;
  }

  /**
   * Возвращает тип изменения
   * @return тип изменения
   */
  public Type getType() {
    return type;
  }

  public enum Type {
    /**
     * Изменилась какае-то сущность или конфиг
     */
    CHANGE,
    /**
     * Добавилась новая сущность (например, маршрут или политика передоставки)
     */
    ADD,
    /**
     * Какае-то сущность была удалена (например, маршрут или политика передоставки)
     */
    REMOVE,
    /**
     * Запуск сервиса
     */
    SERVICE_START,
    /**
     * Остановка сервиса
     */
    SERVICE_STOP,
    /**
     * Переключанение ноды
     */
    SERVICE_NODE_SWITCH
  }

  @SuppressWarnings({"RedundantIfStatement"})
  @Override
  public boolean equals(Object o) {
    if (this == o) return true;
    if (o == null || getClass() != o.getClass()) return false;

    JournalRecord record = (JournalRecord) o;

    if (time != record.time) return false;
    if (!Arrays.equals(descriptionArgs, record.descriptionArgs)) return false;
    if (descriptionKey != null ? !descriptionKey.equals(record.descriptionKey) : record.descriptionKey != null)
      return false;
    if (subject != null ? !subject.equals(record.subject) : record.subject != null) return false;
    if (type != record.type) return false;
    if (user != null ? !user.equals(record.user) : record.user != null) return false;

    return true;
  }

}
