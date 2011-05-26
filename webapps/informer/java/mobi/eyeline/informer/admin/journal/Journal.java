package mobi.eyeline.informer.admin.journal;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.admin.delivery.Delivery;
import mobi.eyeline.informer.admin.filesystem.FileSystem;
import mobi.eyeline.informer.admin.informer.InformerSettings;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.regions.Region;
import mobi.eyeline.informer.admin.restriction.Restriction;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;
import mobi.eyeline.informer.admin.smsc.Smsc;
import mobi.eyeline.informer.admin.users.User;

import java.io.File;
import java.util.Arrays;
import java.util.List;

/**
 * Журнал операций Informer
 *
 * @author Aleksandr Khalitov
 */
public class Journal {


  private final UserSettingsDiffHelper users = new UserSettingsDiffHelper();

  private final InformerSettingsDiffHelper informerSettings = new InformerSettingsDiffHelper();

  private final BlackListDiffHelper blacklist = new BlackListDiffHelper();

  private final SmscDiffHelper smsc = new SmscDiffHelper();

  private final RegionsDiffHelper regions = new RegionsDiffHelper();

  private final InformerDiffHelper informer = new InformerDiffHelper();

  private final DeliveriesDiffHelper deliveries = new DeliveriesDiffHelper();

  private final RestrictionsDiffHelper restrictions = new RestrictionsDiffHelper();

  private final WebconfigDiffHelper webconfig = new WebconfigDiffHelper();

  private final FtpServerDiffHelper ftpserver = new FtpServerDiffHelper();
  private final ArchiveDaemonDiffHelper archiveDaemon = new ArchiveDaemonDiffHelper();
  private final PvssDiffHelper pvss = new PvssDiffHelper();

  private final JournalDataSource ds;

  public Journal(File journalDir, FileSystem fs) throws AdminException {
    this.ds = new JournalFileDataSource(journalDir, fs);
  }

  /**
   * Возвращает cписок всех возможных сабжектов
   *
   * @return список всех возможных сабжектов
   */
  public List<Subject> getSubjects() {
    return Arrays.asList(Subject.values());
  }

  /**
   * Возвращает все записи из журнала, удовлетворяющие фильтру (ограничение: 1000 записей)
   *
   * @param filter фильтр записей
   * @param visitor визитер   *
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка при извлечении записей
   */
  public void getRecords(JournalFilter filter, JournalVisitor visitor) throws AdminException {
    ds.visit(filter, visitor);
  }

  /**
   * Просматривает все записи из журнала, удовлетворяющие фильтру
   *
   * @param filter  фильтр записей
   * @param visitor посетитель
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка при извлечении записей
   */
  public void visit(JournalFilter filter, JournalVisitor visitor) throws AdminException {
    ds.visit(filter, visitor);
  }


  /**
   * Добавляет в журнал новую запись.
   *
   * @param type        тип записи
   * @param subject     субъект
   * @param user        пользователь
   * @param description ключ описания
   * @param args        параметры описания
   * @throws mobi.eyeline.informer.admin.AdminException
   *          Ошибка при сохранении записи
   */
  void addRecord(JournalRecord.Type type, Subject subject, String user, String description, String... args) throws AdminException {
    ds.addRecords(new JournalRecord(type).
        setSubject(subject).setUser(user).setDescription(description, args));
  }


  /**
   * Ищет различия между настройками Informer и записывает их в журнал
   *
   * @param oldSettings старые настройки пользователей
   * @param newSettings новые настройки пользователей
   * @param user        пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка сохранения записи
   */
  public void logChanges(InformerSettings oldSettings, InformerSettings newSettings, String user) throws AdminException {
    informerSettings.logChanges(this, oldSettings, newSettings, user);
  }

  /**
   * Добавляет в журнал запись о новом запрещённом номере
   *
   * @param address адрес
   * @param user    пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка сохранения записи
   */
  public void logAddBlacklist(String address, String user) throws AdminException {
    blacklist.logAddBlacklist(this, address, user);
  }

  /**
   * Добавляет в журнал запись об удалении из списка запрещённых номеров
   *
   * @param address адрес
   * @param user    пользователь, от имени которого надо формировать записи
   * @throws mobi.eyeline.informer.admin.AdminException
   *          ошибка сохранения записи
   */
  public void logRemoveBlacklist(String address, String user) throws AdminException {
    blacklist.logRemoveBlacklist(this, address, user);
  }


  /**
   * Добавляет в журнал запись о новом регионе
   *
   * @param region регион
   * @param user   пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logAddRegion(String region, String user) throws AdminException {
    this.regions.logAddRegion(region, this, user);
  }

  /**
   * Добавляет в журнал запись об обновлении конфигурации регионов
   *
   * @param user   пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logUpdateAllRegions(String user) throws AdminException {
    this.regions.logUpdateAll(this, user);
  }

  /**
   * Добавляет в журнал запись об удалении региона
   *
   * @param region регион
   * @param user   пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logRemoveRegion(String region, String user) throws AdminException {
    this.regions.logRemoveRegion(region, this, user);
  }

  /**
   * Добавляет в журнал запись об обновлении региона
   *
   * @param oldRegion старый регион
   * @param newRegion новый регион
   * @param user      пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logUpdateRegion(Region oldRegion, Region newRegion, String user) throws AdminException {
    this.regions.logChanges(oldRegion, newRegion, this, user);
  }

  /**
   * Добавляет в журнал запись об изменении макс. кол-ва смс по умолчанию
   *
   * @param oldValue старое значение
   * @param newValue новое значение
   * @param user     пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logSetDefaultSmsPerSecondRegion(int oldValue, int newValue, String user) throws AdminException {
    this.regions.logSetDefault(this, oldValue, newValue, user);
  }

  /**
   * Добавляет в журнал запись о новом СМСЦ
   *
   * @param smsc СМСЦ
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logAddSmsc(String smsc, String user) throws AdminException {
    this.smsc.logAddSmsc(smsc, this, user);
  }

  /**
   * Добавляет в журнал запись об удалении СМСЦ
   *
   * @param smsc СМСЦ
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logRemoveSmsc(String smsc, String user) throws AdminException {
    this.smsc.logRemoveSmsc(smsc, this, user);
  }

  /**
   * Добавляет в журнал запись об обновлении СМСЦ
   *
   * @param oldSmsc старый СМСЦ
   * @param newSmsc новый СМСЦ
   * @param user    пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logUpdateSmsc(Smsc oldSmsc, Smsc newSmsc, String user) throws AdminException {
    this.smsc.logChanges(oldSmsc, newSmsc, this, user);
  }

  /**
   * Добавляет в журнал запись об изменении СМСЦ по умолчанию СМСЦ
   *
   * @param oldSmsc старый СМСЦ
   * @param newSmsc новый СМСЦ
   * @param user    пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logSetDefaultSmsc(String oldSmsc, String newSmsc, String user) throws AdminException {
    this.smsc.logSetDefault(oldSmsc, newSmsc, this, user);
  }

  /**
   * Добавляет в журнал запись о старте FTP server
   *
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logFtpServerStart(String user) throws AdminException {
    ftpserver.logFtpServerStart(this, user);
  }

  /**
   * Добавляет в журнал запись об остановке Ftp server
   *
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logFtpServerStop(String user) throws AdminException {
    ftpserver.logFtpServerStop(this, user);
  }

  /**
   * Добавляет в журнал запись о переключении Ftp server на другой хост
   *
   * @param toHost новый хост
   * @param user   пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logFtpServerSwitch(String toHost, String user) throws AdminException {
    ftpserver.logFtpServerSwitch(this, toHost, user);
  }
  /**
   * Добавляет в журнал запись о старте ArchiveDaemon
   *
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logArchiveDaemonStart(String user) throws AdminException {
    archiveDaemon.logArchiveDaemonStart(this, user);
  }

  /**
   * Добавляет в журнал запись об остановке ArchiveDaemon
   *
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logArchiveDaemonStop(String user) throws AdminException {
    archiveDaemon.logArchiveDaemonStop(this, user);
  }

  /**
   * Добавляет в журнал запись о переключении ArchiveDaemon на другой хост
   *
   * @param toHost новый хост
   * @param user   пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logArchiveDaemonSwitch(String toHost, String user) throws AdminException {
    archiveDaemon.logArchiveDaemonSwitch(this, toHost, user);
  }

  /**
   * Добавляет в журнал запись о старте Pvss
   *
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logPvssStart(String user) throws AdminException {
    pvss.logPvssStart(this, user);
  }

  /**
   * Добавляет в журнал запись об остановке Pvss
   *
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logPvssStop(String user) throws AdminException {
    pvss.logPvssStop(this, user);
  }

  /**
   * Добавляет в журнал запись о переключении Pvss на другой хост
   *
   * @param toHost новый хост
   * @param user   пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logPvssSwitch(String toHost, String user) throws AdminException {
    pvss.logPvssSwitch(this, toHost, user);
  }

  /**
   * Добавляет в журнал запись о старте Informer
   *
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logInformerStart(String user) throws AdminException {
    informer.logInformerStart(this, user);
  }

  /**
   * Добавляет в журнал запись об остановке Informer
   *
   * @param user пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logInformerStop(String user) throws AdminException {
    informer.logInformerStop(this, user);
  }

  /**
   * Добавляет в журнал запись о переключении Informer на другой хост
   *
   * @param toHost новый хост
   * @param user   пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logInformerSwitch(String toHost, String user) throws AdminException {
    informer.logInformerSwitch(this, toHost, user);
  }

  /**
   * Добавляет в журнал запись о новом пользователе
   *
   * @param login login
   * @param user  пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logAddUser(String login, String user) throws AdminException {
    this.users.logAddUser(login, this, user);
  }

  /**
   * Добавляет в журнал запись об удалении пользователя
   *
   * @param login login
   * @param user  пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logRemoveUser(String login, String user) throws AdminException {
    this.users.logRemoveUser(login, this, user);
  }

  /**
   * Добавляет в журнал запись об обновлении пользователя
   *
   * @param oldUser старый регион
   * @param newUser новый регион
   * @param user    пользователь, от имени которого надо формировать записи
   * @throws AdminException ошибка сохранения записи
   */
  public void logUpdateUser(User oldUser, User newUser, String user) throws AdminException {
    this.users.logChanges(oldUser, newUser, this, user);
  }


  public void logDeliveryPaused(String user, int id) throws AdminException {
    deliveries.logDeliveryPaused(this, user, id);
  }

  public void logDeliveryActivated(String user, int id) throws AdminException {
    deliveries.logDeliveryActivated(this, user, id);
  }

  public void logDeliveriesChanges(String user, Delivery oldDelivery, Delivery newDelivery) throws AdminException {
    deliveries.logChanges(this, user, oldDelivery, newDelivery);
  }

  public void logDeliveryCreated(String user, Delivery d) throws AdminException {
    deliveries.logDeliveryCreated(this, user, d.getId(), d.getName());
  }


  public void logDeliveryDroped(String user, Delivery d) throws AdminException {
    deliveries.logDeliveryDroped(this, user, d.getId(), d.getName());
  }

  public void logDeliveryArchivated(String user, Delivery d) throws AdminException {
    deliveries.logDeliveryArchivated(this, user, d.getId(), d.getName());
  }

  public void logAddRestriction(Restriction r, String user) throws AdminException {
    restrictions.logRestrictionAdd(this, user, r);
  }

  public void logUpdateRestriction(Restriction r, Restriction oldR, String user) throws AdminException {
    restrictions.logUpdateRestriction(this, user, r, oldR);
  }

  public void logDeleteRestriction(Restriction r, String user) throws AdminException {
    restrictions.logDeleteRestriction(this, user, r);
  }

  public void logUpdateNotificationSettings(NotificationSettings props, NotificationSettings old, String user) throws AdminException {
    webconfig.logUpdateNotificationSettings(this, props, old, user);
  }

  public void logUpdateSiebelProps(SiebelSettings props, SiebelSettings old, String user) throws AdminException {
    webconfig.logUpdateSiebelProps(this, props, old, user);
  }

  public void logUpdateCdrProps(CdrSettings props, CdrSettings old, String user) throws AdminException {
    webconfig.logUpdateCdrProps(this, props, old, user);
  }
}