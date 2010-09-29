package ru.novosoft.smsc.web.journal;

import java.util.Locale;
import java.util.ResourceBundle;

/**
 * Структура, описывающая одну запись в журнале
 *
 * @author Artem Snopkov
 */
public class JournalRecord {

  private long time;
  private Subject subjectKey;
  private String user;
  private final Type type;
  private String descriptionKey;
  private String[] descriptionArgs;

  public JournalRecord(Type type) {
    this.type = type;
  }

  /**
   * Возвращает время в миллисекундах, в которое произошло изменение
   *
   * @return время в миллисекундах, в которое произошло изменение
   */
  public long getTime() {
    return time;
  }

  void setTime(long time) {
    this.time = time;
  }

  /**
   * Возвращает название сабжекта в указанной локали
   *
   * @param locale локаль
   * @return название сабжекта в указанной локали
   */
  public String getSubject(Locale locale) {
    return ResourceBundle.getBundle(JournalRecord.class.getCanonicalName(), locale).getString(subjectKey.getKey());
  }

  Subject getSubjectKey() {
    return subjectKey;
  }

  void setSubjectKey(Subject subject) {
    this.subjectKey = subject;
  }

  /**
   * Возвращает логин пользователь, произвевшего изменение
   *
   * @return логин пользователь, произвевшего изменение
   */
  public String getUser() {
    return user;
  }

  void setUser(String user) {
    this.user = user;
  }

  public void setDescription(String key, String... args) {
    this.descriptionKey = key;
    this.descriptionArgs = args;
  }

  String getDescriptionKey() {
    return descriptionKey;
  }

  String[] getDescriptionArgs() {
    return descriptionArgs;
  }

  /**
   * Возвращает описание изменения в заданной локали
   *
   * @param locale локаль
   * @return описание изменения в заданной локали
   */
  public String getDescription(Locale locale) {
    String str = ResourceBundle.getBundle(JournalRecord.class.getCanonicalName(), locale).getString(descriptionKey);
    if (descriptionArgs != null) {
      for (int i = 0; i < descriptionArgs.length; i++)
        str = str.replaceAll("\\{" + i + "\\}", descriptionArgs[i]);
    }
    return str;
  }

  /**
   * Возвращает тип изменения
   *
   * @return тип изменения
   */
  public Type getType() {
    return type;
  }

  public enum Subject {
    SMSC("subject.smsc"),
    RESCHEDULE("subject.reschedule"),
    CLOSED_GROUPS("subject.closed_groups"),
    USERS("subject.user"),
    MAP_LIMIT("subject.maplimit"),
    LOGGING("subject.logging"),
    FRAUD("subject.fraud"),
    SNMP("subject.snmp"),
    SME("subject.sme"),
    ACL("subject.acl"),
    CATEGORY("subject.category");

    private final String key;

    Subject(String key) {
      this.key = key;
    }

    public String getKey() {
      return key;
    }
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
     * Запуск какого-то сервиса
     */
    START,
    /**
     * Остановка какого-то сервиса
     */
    STOP,
    /**
     * Отключение SME от СМСЦ
     */
    DISCONNECT,
    /**
     * Переключение сервиса с ноды на ноду
     */
    SWITCH
  }
}
