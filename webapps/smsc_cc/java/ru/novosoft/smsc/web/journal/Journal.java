package ru.novosoft.smsc.web.journal;

import java.util.*;

/**
 * Журнал, представляет собой коллекцию объектов класса JournalRecord.
 * @author Artem Snopkov
 */
public class Journal {

  private final List<JournalRecord> records = new ArrayList<JournalRecord>();

  /**
   * Возвращает список всех возможных сабжектов в указанной локали
   * @param locale локаль
   * @return список всех возможных сабжектов в указанной локали
   */
  public List<String> getSubjects(Locale locale) {
    List<String> l = new ArrayList<String>();
    ResourceBundle rb = ResourceBundle.getBundle(JournalRecord.class.getCanonicalName(), locale);
    for (JournalRecord.Subject s : JournalRecord.Subject.values())
      l.add(rb.getString("subject." + s.getKey()));
    return l;
  }

  /**
   * Возвращает все записи из журнала
   * @return все записи из журнала
   */
  public synchronized List<JournalRecord> getRecords() {
    return new ArrayList<JournalRecord>(records);
  }

  /**
   * Проверяет журнал на пустоту
   * @return true, если журнал пуст. Иначе возвращает false.
   */
  public boolean isEmpty() {
    return records.isEmpty();
  }

  /**
   * Возвращает количество записей в журнала
   * @return количество записей в журнала
   */
  public int size() {
    return records.size();
  }

  /**
   * Возвращает все записи с указанным субъектом
   * @param subject субъект, для которого надо вернуть записи
   * @return все записи с указанным субъектом
   */
  public synchronized List<JournalRecord> getRecords(JournalRecord.Subject subject) {
    List<JournalRecord> res = new ArrayList<JournalRecord>();
    for (JournalRecord r : records) {
      if (r.getSubjectKey() == subject)
        res.add(r);
    }
    return res;
  }

  /**
   * Удаляет из лога все записи с указанным сабжектом
   *
   * @param subject сабжект, записи которого надо удалить
   */
  public synchronized void removeRecords(JournalRecord.Subject subject) {
    for (Iterator<JournalRecord> i = records.iterator(); i.hasNext();) {
      JournalRecord r = i.next();
      if (r.getSubjectKey() == subject)
        i.remove();
    }
  }

  /**
   * Проверяет наличие в журнале записей с указанным Subject
   * @param subject субъект
   * @return true, если в журнале имеются записи с данным subject. Иначе возвращает false
   */
  public synchronized boolean hasRecords(JournalRecord.Subject subject) {
    for (JournalRecord r : records) {
      if (r.getSubjectKey() == subject)
        return true;
    }
    return false;
  }

  /**
   * Добавляет в журнал новую запись с указанными параметрами и возвращает указатель на нее.
   * @param type тип записи
   * @param subject субъект
   * @param user пользователь
   * @return новую запись
   */
  public synchronized JournalRecord addRecord(JournalRecord.Type type, JournalRecord.Subject subject, String user) {
    JournalRecord r = new JournalRecord(type);
    r.setUser(user);
    r.setTime(System.currentTimeMillis());
    r.setSubjectKey(subject);

    records.add(r);
    return r;
  }

  public UserJournal user(String user) {
    return new UserJournal(user, this);
  }

}