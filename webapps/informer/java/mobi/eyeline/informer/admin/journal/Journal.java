package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.UsersSettings;

import java.io.File;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;

/**
 * Журнал операций Informer
 * @author Aleksandr Khalitov
 */
public class Journal {


  private final UserSettingsDiffHelper users = new UserSettingsDiffHelper();

  private final InformerSettingsDiffHelper informer = new InformerSettingsDiffHelper();

  private final SmscDiffHelper smsc = new SmscDiffHelper();

  private final RegionsDiffHelper regions = new RegionsDiffHelper();

  private final JournalDataSource ds;

  public Journal(File journalDir, FileSystem fs) throws AdminException{
    this.ds = new JournalFileDataSource(journalDir, fs);
  }

  /**
   * Возвращает cписок всех возможных сабжектов
   * @return список всех возможных сабжектов
   */
  public List<Subject> getSubjects() {
    return Arrays.asList(Subject.values());
  }

  /**
   * Возвращает все записи из журнала, удовлетворяющие фильтру
   * @param filter фильтр записей
   * @return все записи из журнала, удовлетворяющие фильтру
   * @throws mobi.eyeline.informer.admin.AdminException ошибка при извлечении записей
   */
  public List<JournalRecord> getRecords(JournalFilter filter) throws AdminException {
    final List<JournalRecord> records = new LinkedList<JournalRecord>();
    ds.visit(filter, new JournalVisitor() {
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
  public void visit(JournalFilter filter, JournalVisitor visitor) throws AdminException {
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
  void addRecord(JournalRecord.Type type, Subject subject, String user, String description, String ... args) throws AdminException {
    ds.addRecords(new JournalRecord(type).
        setSubject(subject).setUser(user).setDescription(description, args));
  }

  /**
   * Ищет различия между настройками пользователей и записывает их в журнал
   * @param oldSettings старые настройки пользователей
   * @param newSettings новые настройки пользователей
   * @param user пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException ошибка сохранения записи
   */
  public void logChanges(UsersSettings oldSettings, UsersSettings newSettings, String user) throws AdminException {
    users.logChanges(this, oldSettings, newSettings, user);
  }

  /**
   * Ищет различия между настройками Informer и записывает их в журнал
   * @param oldSettings старые настройки пользователей
   * @param newSettings новые настройки пользователей
   * @param user пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException ошибка сохранения записи
   */
  public void logChanges(InformerSettings oldSettings, InformerSettings newSettings, String user) throws AdminException {
    informer.logChanges(this, oldSettings, newSettings, user);
  }

  /**
   * Добавляет в журнал запись о новом запрещённом номере
   * @param address адрес
   * @param user пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException ошибка сохранения записи
   */
  public void logAddBlacklist(String address, String user) throws AdminException {
    addRecord(JournalRecord.Type.ADD, Subject.BLACKLIST, user, "blacklist_added", address);
  }
  /**
   * Добавляет в журнал запись об удалении из списка запрещённых номеров
   * @param address адрес
   * @param user пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException ошибка сохранения записи
   */
  public void logRemoveBlacklist(String address, String user) throws AdminException {
    addRecord(JournalRecord.Type.REMOVE, Subject.BLACKLIST, user, "blacklist_removed", address);
  }


  /**
   * Добавляет в журнал запись о новом регионе
   * @param region регион
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logAddRegion(String region, String user) throws AdminException{
    this.regions.logAddRegion(region, this, user);
  }

  /**
   * Добавляет в журнал запись об удалении региона
   * @param region регион
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logRemoveRegion(String region, String user) throws AdminException{
    this.regions.logRemoveRegion(region, this, user);
  }

  /**
   * Добавляет в журнал запись об обновлении региона
   * @param oldRegion старый регион
   * @param newRegion новый регион
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logUpdateRegion(Region oldRegion, Region newRegion, String user) throws AdminException{
    this.regions.logChanges(oldRegion, newRegion, this, user);
  }

  /**
   * Добавляет в журнал запись о новом СМСЦ
   * @param smsc СМСЦ
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logAddSmsc(String smsc, String user) throws AdminException{
    this.smsc.logAddSmsc(smsc, this, user);
  }

  /**
   * Добавляет в журнал запись об удалении СМСЦ
   * @param smsc СМСЦ
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logRemoveSmsc(String smsc, String user) throws AdminException{
    this.smsc.logRemoveSmsc(smsc, this, user);
  }

  /**
   * Добавляет в журнал запись об обновлении СМСЦ
   * @param oldSmsc старый СМСЦ
   * @param newSmsc новый СМСЦ
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logUpdateSmsc(Smsc oldSmsc, Smsc newSmsc, String user) throws AdminException{
    this.smsc.logChanges(oldSmsc, newSmsc, this, user);
  }

  /**
   * Добавляет в журнал запись об изменении СМСЦ по умолчанию СМСЦ
   * @param oldSmsc старый СМСЦ
   * @param newSmsc новый СМСЦ
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logSetDefaultSmsc(String oldSmsc, String newSmsc, String user) throws AdminException{
    this.smsc.logSetDefault(oldSmsc, newSmsc, this, user);
  }

}