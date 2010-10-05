package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.category.CategorySettings;
import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.admin.logging.LoggerSettings;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.sme.Sme;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.snmp.SnmpSettings;
import ru.novosoft.smsc.admin.users.UsersSettings;
import ru.novosoft.smsc.util.Address;

import java.util.*;

/**
 * Журнал, представляет собой коллекцию объектов класса JournalRecord.
 * @author Artem Snopkov
 */
public class Journal {

  private final List<JournalRecord> records = new ArrayList<JournalRecord>();

  private final SmscSettingsDiffHelper smsc = new SmscSettingsDiffHelper(JournalRecord.Subject.SMSC);
  private final RescheduleSettingsDiffHelper reschedule = new RescheduleSettingsDiffHelper(JournalRecord.Subject.RESCHEDULE);
  private final UserSettingsDiffHelper users = new UserSettingsDiffHelper(JournalRecord.Subject.USERS);
  private final SmeDiffHelper sme = new SmeDiffHelper(JournalRecord.Subject.SME);

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


  // SME

  public void logSmeAdded(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.ADD, JournalRecord.Subject.SME, user);
    r.setDescription("sme.added", smeId);
  }

  public void logSmeUpdated(Sme oldSme, Sme newSme, String user) {
    sme.logChanges(this, oldSme, newSme, user);
  }

  public void logSmeRemoved(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.REMOVE, JournalRecord.Subject.SME, user);
    r.setDescription("sme.removed", smeId);
  }

  public void logSmeDisconnected(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.DISCONNECT, JournalRecord.Subject.SME, user);
    r.setDescription("sme.disconnected", smeId);
  }

  public void logSmeStopped(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.STOP, JournalRecord.Subject.SME, user);
    r.setDescription("sme.stopped", smeId);
  }

  public void logSmeStarted(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.START, JournalRecord.Subject.SME, user);
    r.setDescription("sme.started", smeId);
  }

  public void logSmeSwitched(String smeId, String toHost, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.SWITCH, JournalRecord.Subject.SME, user);
    r.setDescription("sme.switched", smeId, toHost);
  }  

}