package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.retry_policies.RetryPolicy;
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

  private final InformerSettingsDiffHelper informerSettings = new InformerSettingsDiffHelper();

  private final BlackListDiffHelper blacklist = new BlackListDiffHelper();

  private final SmscDiffHelper smsc = new SmscDiffHelper();

  private final RegionsDiffHelper regions = new RegionsDiffHelper();

  private final RetryPolicyDiffHelper retryPolicy = new RetryPolicyDiffHelper(Subject.RETRY_POLICY);

  private final InformerDiffHelper informer = new InformerDiffHelper();

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
   * Возвращает все записи из журнала, удовлетворяющие фильтру (ограничение: 1000 записей)
   * @param filter фильтр записей
   * @return все записи из журнала, удовлетворяющие фильтру
   * @throws mobi.eyeline.informer.admin.AdminException ошибка при извлечении записей
   */
  public List<JournalRecord> getRecords(JournalFilter filter) throws AdminException {
    final LinkedList<JournalRecord> records = new LinkedList<JournalRecord>();
    ds.visit(filter, new JournalVisitor() {
      private static final int LIMIT = 1000;
      public boolean visit(JournalRecord r) {
        if(records.size() == LIMIT) {
          records.removeFirst();
        }
        records.addLast(r);
        return true;
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
    informerSettings.logChanges(this, oldSettings, newSettings, user);
  }

  /**
   * Добавляет в журнал запись о новом запрещённом номере
   * @param address адрес
   * @param user пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException ошибка сохранения записи
   */
  public void logAddBlacklist(String address, String user) throws AdminException {
    blacklist.logAddBlacklist(this, address, user);
  }
  /**
   * Добавляет в журнал запись об удалении из списка запрещённых номеров
   * @param address адрес
   * @param user пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException ошибка сохранения записи
   */
  public void logRemoveBlacklist(String address, String user) throws AdminException {
    blacklist.logRemoveBlacklist(this, address, user);
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
   * Добавляет в журнал запись об изменении макс. кол-ва смс по умолчанию
   * @param oldValue старое значение
   * @param newValue новое значение
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logSetDefaultSmsPerSecondRegion(int oldValue, int newValue, String user) throws AdminException{
    this.regions.logSetDefault(this, oldValue, newValue, user);
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

  public void logUpdateRetryPolicy(RetryPolicy rpOld, RetryPolicy rp, String user) throws AdminException {
    this.retryPolicy.logUpdateRetryPolicy(this, rpOld, rp, user);
  }
  public void logAddRetryPolicy( RetryPolicy rp, String user) throws AdminException {
    this.retryPolicy.logAddRetryPolicy(this,  rp, user);
  }
  public void logRemoveRetryPolicy( String policyId, String user) throws AdminException {
    this.retryPolicy.logRemoveRetryPolicy(this,  policyId, user);
  }

  /**
   * Добавляет в журнал запись о старте Informer
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logInformerStart(String user) throws AdminException {
    informer.logInformerStart(this, user);
  }

  /**
   * Добавляет в журнал запись об остановке Informer
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logInformerStop(String user) throws AdminException {
    informer.logInformerStop(this, user);
  }

  /**
   * Добавляет в журнал запись о переключении Informer на другой хост
   * @param toHost новый хост
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logInformerSwitch(String toHost, String user) throws AdminException {
    informer.logInformerSwitch(this, toHost, user);
  }

  /**
   * Добавляет в журнал запись о старте демона
   * @param name имя демона
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logDaemonrStart(String name, String user) throws AdminException {
    addRecord(JournalRecord.Type.SERVICE_START, Subject.DAEMONS, user, "daemon_start", name);
  }

  /**
   * Добавляет в журнал запись об остановке демона
   * @param name имя демона
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logDaemonStop(String name, String user) throws AdminException {
    addRecord(JournalRecord.Type.SERVICE_STOP, Subject.DAEMONS, user, "daemon_stop", name);
  }
}