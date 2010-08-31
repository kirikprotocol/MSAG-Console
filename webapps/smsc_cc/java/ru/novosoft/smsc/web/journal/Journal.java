package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.users.UsersSettings;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * Журнал, представляет собой коллекцию объектов класса JournalRecord.
 * @author Artem Snopkov
 */
public class Journal {

  public static final String SMSC = "subject.smsc";
  public static final String RESCHEDULE = "subject.reschedule";
  public static final String USERS = "subject.user";
  public static final String MAP_LIMIT = "subject.maplimit";

  private final List<JournalRecord> records = new ArrayList<JournalRecord>();

  private final SmscSettingsDiffHelper smsc = new SmscSettingsDiffHelper(SMSC);
  private final RescheduleSettingsDiffHelper reschedule = new RescheduleSettingsDiffHelper(RESCHEDULE);
  private final UserSettingsDiffHelper users = new UserSettingsDiffHelper(USERS);
  private final MapLimitSettingsDiffHelper mapLimit = new MapLimitSettingsDiffHelper(MAP_LIMIT);


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
   * Возвращает последнюю запись для указанного subject, с датой после afterTime или null, если такой нет.
   *
   * @param subject   сабжект
   * @param afterTime время, начиная с которого надо искать запись
   * @return последнюю запись для указанного subject, с датой после afterTime или null, если такой нет.
   */
  public synchronized JournalRecord getLastRecord(String subject, long afterTime) {
    for (int i = records.size() - 1; i >= 0; i--) {
      JournalRecord r = records.get(i);
      if (r.getSubjectKey().equals(subject)) {
        if (r.getTime() > afterTime)
          return r;
        else
          return null;
      }
    }
    return null;
  }

  /**
   * Возвращает все записи с указанным субъектом
   * @param subject субъект, для которого надо вернуть записи
   * @return все записи с указанным субъектом
   */
  public synchronized List<JournalRecord> getRecords(String subject) {
    List<JournalRecord> res = new ArrayList<JournalRecord>();
    for (JournalRecord r : records) {
      if (r.getSubjectKey().equals(subject))
        res.add(r);
    }
    return res;
  }

  /**
   * Удаляет из лога все записи с указанным сабжектом
   *
   * @param subject сабжект, записи которого надо удалить
   */
  public synchronized void removeRecords(String subject) {
    for (Iterator<JournalRecord> i = records.iterator(); i.hasNext();) {
      JournalRecord r = i.next();
      if (r.getSubjectKey().equals(subject))
        i.remove();
    }
  }

  /**
   * Проверяет наличие в журнале записей с указанным Subject
   * @param subject субъект
   * @return true, если в журнале имеются записи с данным subject. Иначе возвращает false
   */
  public synchronized boolean hasRecords(String subject) {
    for (JournalRecord r : records) {
      if (r.getSubjectKey().equals(subject))
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
  synchronized JournalRecord addRecord(JournalRecord.Type type, String subject, String user) {
    JournalRecord r = new JournalRecord(type);
    r.setUser(user);
    r.setTime(System.currentTimeMillis());
    r.setSubjectKey(subject);

    records.add(r);
    return r;
  }

  /**
   * Ищет различия между настройками СМСЦ и записывает их в журнал
   * @param oldSettings старые настройки СМСЦ
   * @param newSettings новые настройки СМСЦ
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logChanges(SmscSettings oldSettings, SmscSettings newSettings, String user) {
    smsc.logChanges(this, oldSettings, newSettings, user);
  }

  /**
   * Ищет различия между настройками политик передоставки и записывает их в журнал
   * @param oldSettings старые политики передоставки
   * @param newSettings новые политики передоставки
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logChanges(RescheduleSettings oldSettings, RescheduleSettings newSettings, String user) {
    reschedule.logChanges(this, oldSettings, newSettings, user);
  }

  /**
   * Ищет различия между настройками пользователей и записывает их в журнал
   * @param oldSettings старые настройки пользователей
   * @param newSettings новые настройки пользователей
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logChanges(UsersSettings oldSettings, UsersSettings newSettings, String user) {
    users.logChanges(this, oldSettings, newSettings, user);
  }

  /**
   * Ищет различия между настройками Map Limits и записывает их в журнал
   * @param oldSettings старые настройки Map Limits
   * @param newSettings новые настройки Map Limits
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logChanges(MapLimitSettings oldSettings, MapLimitSettings newSettings, String user) {
    mapLimit.logChanges(this, oldSettings, newSettings, user);
  }
}
