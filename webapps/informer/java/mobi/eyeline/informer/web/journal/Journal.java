package mobi.eyeline.informer.web.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.web.users.UsersSettings;

import java.util.*;

/**
 * Журнал операций Informer
 * @author Aleksandr Khalitov
 */
public class Journal {

  public static final String USERS = "subject.user";


  private final UserSettingsDiffHelper users = new UserSettingsDiffHelper(USERS);

  private final JournalDataSource ds;

  public Journal(JournalDataSource ds) {
    this.ds = ds;
  }

  /**
   * Возвращает map всех возможных сабжектов в указанной локали
   * @param locale локаль
   * @return map всех возможных сабжектов в указанной локали
   */
  public Map<String, String> getSubjects(Locale locale) {
    Map<String, String> l = new TreeMap<String, String>();
    ResourceBundle rb = ResourceBundle.getBundle(JournalRecord.class.getCanonicalName(), locale);
    l.put(USERS, rb.getString(USERS));
    return l;
  }

  /**
   * Возвращает все записи из журнала, удовлетворяющие фильтру
   * @param filter фильтр записей
   * @return все записи из журнала, удовлетворяющие фильтру
   * @throws mobi.eyeline.informer.admin.AdminException ошибка при извлечении записей
   */
  public List<JournalRecord> getRecords(JournalFilter filter) throws AdminException {
    final List<JournalRecord> records = new LinkedList<JournalRecord>();
    ds.visit(filter, new JournalDataSource.Visitor() {
      private static final int LIMIT = 1000;
      private int count = 0;
      public boolean visit(JournalRecord r) {
        records.add(r);
        count++;
        return count != LIMIT;
      }
    });
    return records;
  }

  /**
   * Просматривает все записи из журнала, удовлетворяющие фильтру
   * @param filter фильтр записей
   * @param visitor посетитель
   * @throws mobi.eyeline.informer.admin.AdminException ошибка при извлечении записей
   */
  public void visit(JournalFilter filter, JournalDataSource.Visitor visitor) throws AdminException {
    ds.visit(filter, visitor);
  }


  /**
   * Добавляет в журнал новую запись.
   * @param type тип записи
   * @param subject субъект
   * @param user пользователь
   * @param description ключ описания
   * @param args параметры описания
   * @throws mobi.eyeline.informer.admin.AdminException Ошибка при сохранении записи
   */
  void addRecord(JournalRecord.Type type, String subject, String user, String description, String ... args) throws AdminException {
    ds.addRecords(new JournalRecord(type).setTime(System.currentTimeMillis()).
        setSubjectKey(subject).setUser(user).setDescription(description, args));
  }
//
//  public void logChangesForObjects(Object oldObj, Object newObj, String user) {
//    if (oldObj instanceof SmscSettings)
//      logChanges((SmscSettings)oldObj, (SmscSettings)newObj, user);
//    else if (oldObj instanceof RescheduleSettings)
//      logChanges((RescheduleSettings)oldObj, (RescheduleSettings)newObj, user);
//    else if (oldObj instanceof UsersSettings)
//      logChanges((UsersSettings)oldObj, (UsersSettings)newObj, user);
//    else if (oldObj instanceof MapLimitSettings)
//      logChanges((MapLimitSettings)oldObj, (MapLimitSettings)newObj, user);
//  }

  /**
   * Ищет различия между настройками пользователей и записывает их в журнал
   * @param oldSettings старые настройки пользователей
   * @param newSettings новые настройки пользователей
   * @param user пользователь, от имени которого надо формировать записи
   */
  public void logChanges(UsersSettings oldSettings, UsersSettings newSettings, String user) throws AdminException {
    users.logChanges(this, oldSettings, newSettings, user);
  }

}