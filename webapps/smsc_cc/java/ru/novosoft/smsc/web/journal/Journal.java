package ru.novosoft.smsc.web.journal;

import ru.novosoft.smsc.admin.fraud.FraudSettings;
import ru.novosoft.smsc.admin.logging.LoggerSettings;
import ru.novosoft.smsc.admin.map_limit.MapLimitSettings;
import ru.novosoft.smsc.admin.reschedule.RescheduleSettings;
import ru.novosoft.smsc.admin.sme.Sme;
import ru.novosoft.smsc.admin.smsc.SmscSettings;
import ru.novosoft.smsc.admin.snmp.SnmpSettings;
import ru.novosoft.smsc.admin.users.UsersSettings;

import java.util.*;

/**
 * Журнал, представляет собой коллекцию объектов класса JournalRecord.
 * @author Artem Snopkov
 */
public class Journal {

  public static final String SMSC = "subject.smsc";
  public static final String RESCHEDULE = "subject.reschedule";
  public static final String CLOSED_GROUPS = "subject.closed_groups";
  public static final String USERS = "subject.user";
  public static final String MAP_LIMIT = "subject.maplimit";
  public static final String FRAUD = "subject.fraud";
  public static final String SNMP="subject.snmp";
  public static final String SME="subject.sme";

  private final List<JournalRecord> records = new ArrayList<JournalRecord>();

  private final SmscSettingsDiffHelper smsc = new SmscSettingsDiffHelper(SMSC);
  private final RescheduleSettingsDiffHelper reschedule = new RescheduleSettingsDiffHelper(RESCHEDULE);
  private final UserSettingsDiffHelper users = new UserSettingsDiffHelper(USERS);
  private final MapLimitSettingsDiffHelper mapLimit = new MapLimitSettingsDiffHelper(MAP_LIMIT);
  private final LoggerSettingsDiffHelper logger = new LoggerSettingsDiffHelper(MAP_LIMIT);
  private final FraudSettingsDiffHelper fraud = new FraudSettingsDiffHelper(FRAUD);
  private final SnmpSettingsDiffHelper snmp = new SnmpSettingsDiffHelper(SNMP);
  private final SmeDiffHelper sme = new SmeDiffHelper(SME);

  /**
   * Возвращает список всех возможных сабжектов в указанной локали
   * @param locale локаль
   * @return список всех возможных сабжектов в указанной локали
   */
  public List<String> getSubjects(Locale locale) {
    List<String> l = new ArrayList<String>();
    ResourceBundle rb = ResourceBundle.getBundle(JournalRecord.class.getCanonicalName(), locale);
    l.add(rb.getString(SMSC));
    l.add(rb.getString(RESCHEDULE));
    l.add(rb.getString(USERS));
    l.add(rb.getString(MAP_LIMIT));
    l.add(rb.getString(CLOSED_GROUPS));
    l.add(rb.getString(FRAUD));
    l.add(rb.getString(SNMP));
    l.add(rb.getString(SME));
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

  public void logChangesForObjects(Object oldObj, Object newObj, String user) {
    if (oldObj instanceof SmscSettings)
      logChanges((SmscSettings)oldObj, (SmscSettings)newObj, user);
    else if (oldObj instanceof RescheduleSettings)
      logChanges((RescheduleSettings)oldObj, (RescheduleSettings)newObj, user);
    else if (oldObj instanceof UsersSettings)
      logChanges((UsersSettings)oldObj, (UsersSettings)newObj, user);
    else if (oldObj instanceof MapLimitSettings)
      logChanges((MapLimitSettings)oldObj, (MapLimitSettings)newObj, user);
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

  /**
   * Ищет различия между настройками Logger и записывает их в журнал
   * @param oldSettings старые настройки Logger
   * @param newSettings новые настройки Logger
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logChanges(LoggerSettings oldSettings, LoggerSettings newSettings, String user) {
    logger.logChanges(this, oldSettings, newSettings, user);
  }
  
  public void logChanges(FraudSettings oldSettings, FraudSettings newSettings, String user) {
    fraud.logChanges(this, oldSettings, newSettings, user);
  }
  /**
   * Добавляет в журнал запись об изменении описания закрытой группы
   * @param name имя группы
   * @param oldDescription старое описание
   * @param newDescription новое описание
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logClosedGroupDescription(String name, String oldDescription, String newDescription, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.CHANGE, CLOSED_GROUPS, user);
    r.setDescription("closed_group_change_description", oldDescription, newDescription, name);
  }

  /**
   * Добавляет в журнал запись о создании маски закрытой группы
   * @param name имя группы
   * @param mask маска
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logClosedGroupAddMask(String name, String mask, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.CHANGE, CLOSED_GROUPS, user);
    r.setDescription("closed_group_add_mask", mask, name);
  }

  /**
   * Добавляет в журнал запись об удалении маски закрытой группы
   * @param name имя группы
   * @param mask маска
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logClosedGroupRemoveMask(String name, String mask, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.CHANGE, CLOSED_GROUPS, user);
    r.setDescription("closed_group_remove_mask", mask, name);
  }

  /**
   * Добавляет в журнал запись о создании закрытой группы
   * @param name имя группы
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logClosedGroupAdd(String name, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.ADD, CLOSED_GROUPS, user);
    r.setDescription("closed_group_added", name);
  }

  /**
   * Добавляет в журнал запись об удалении закрытой группы 
   * @param name имя группы
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logClosedGroupRemove(String name, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.REMOVE, CLOSED_GROUPS, user);
    r.setDescription("closed_group_removed", name);
  }

  public void logChanges(SnmpSettings oldSettings, SnmpSettings newSettings, String user) {
    snmp.logChanges(this, oldSettings, newSettings, user);
  }

  // SME

  public void logSmeAdded(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.ADD, SME, user);
    r.setDescription("sme.added", smeId);
  }

  public void logSmeUpdated(Sme oldSme, Sme newSme, String user) {
    sme.logChanges(this, oldSme, newSme, user);
  }

  public void logSmeRemoved(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.REMOVE, SME, user);
    r.setDescription("sme.removed", smeId);
  }

  public void logSmeDisconnected(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.DISCONNECT, SME, user);
    r.setDescription("sme.disconnected", smeId);
  }

  public void logSmeStopped(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.STOP, SME, user);
    r.setDescription("sme.stopped", smeId);
  }

  public void logSmeStarted(String smeId, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.START, SME, user);
    r.setDescription("sme.started", smeId);
  }

  public void logSmeSwitched(String smeId, String toHost, String user) {
    JournalRecord r = addRecord(JournalRecord.Type.SWITCH, SME, user);
    r.setDescription("sme.switched", smeId, toHost);
  }


}