package ru.novosoft.smsc.admin;

import ru.novosoft.smsc.util.config.ConfigException;
import ru.novosoft.smsc.util.config.XmlConfig;
import ru.novosoft.smsc.util.config.XmlConfigSection;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

/**
 * @author Artem Snopkov
 */
public class AdminContext {

  private static AdminContext instance;
  private static Lock initLock = new ReentrantLock();

  public static AdminContext getInstance() throws AdminException {
    if (instance == null) {
      try {
        initLock.lock();
        if (instance == null)
          instance = new AdminContext();
      } finally {
        initLock.unlock();
      }
    }
    return instance;
  }

  private final File appBaseDir;
  private final File servicesDir;
  private final XmlConfig webconfig;

  private AdminContext() throws AdminException {
    String appBaseDirStr = System.getProperty("app.base.dir");
    if (appBaseDirStr == null)
      throw new AdminException("System property 'app.base.dir' not found.");

    this.appBaseDir = new File(appBaseDirStr);
    this.servicesDir = new File(appBaseDir, "services");

    this.webconfig = new XmlConfig();
    try {
      this.webconfig.load(new File(appBaseDir, "conf/webconfig.xml"));
    } catch (ConfigException e) {
      throw new AdminException("Unable to load webconfig.xml. Cause: " + e.getMessage(), e);
    }
  }

  /**
   * Возвращает тип установки
   *
   * @return тип установки
   * @throws AdminException если тип установки не задан или задан неверно
   */
  public InstallationType getInstallationType() throws AdminException {
    try {
      String installationTypeStr = webconfig.getSection("installation").getString("type", "single");
      if (installationTypeStr.equalsIgnoreCase("hs"))
        return InstallationType.HS;
      else if (installationTypeStr.equalsIgnoreCase("ha"))
        return InstallationType.HA;
      else if (installationTypeStr.equalsIgnoreCase("single"))
        return InstallationType.SINGLE;
      else
        throw new AdminException("Unknown installation type: " + installationTypeStr);
    } catch (ConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  /**
   * Возвращает базовую директорию приложения
   *
   * @return базовую директорию приложения
   */
  public File getAppBaseDir() {
    return appBaseDir;
  }

  /**
   * Возвращает директорию, в которой находятся дистрибутивы сервисов
   *
   * @return директорию, в которой находятся дистрибутивы сервисов
   */
  public File getServicesDir() {
    return servicesDir;
  }

  // ================================== For Single Installation Type =================================

  /**
   * Возвращает адрес Single Daemon-а.
   * @return адрес Single Daemon-а.
   * @throws AdminException если адрес не задан или InstallationType != SINGLE
   */
  public String getSingleDaemonHost() throws AdminException {
    if (getInstallationType() != InstallationType.SINGLE)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getString("host");
    } catch (ConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  /**
   * Возвращает порт Single Daemon-а.
   * @return порт Single Daemon-а.
   * @throws AdminException если порт не задан или InstallationType != SINGLE
   */
  public int getSingleDaemonPort() throws AdminException {
    if (getInstallationType() != InstallationType.SINGLE)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getInt("port");
    } catch (ConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  // ================================== For HS Installation Type ======================================

  /**
   * Возвращает список зеркал appBaseDir
   * @return список зеркал appBaseDir
   * @throws AdminException если список не задан или InstallationType != HS
   */
  public File[] getAppMirrorDirs() throws AdminException {
    if (getInstallationType() != InstallationType.HS)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection installation = webconfig.getSection("installation");
      File mirrorPath = new File(installation.getString("mirrorpath"));
      return new File[]{mirrorPath};
    } catch (ConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  /**
   * Возвращает адрес HS Daemon-а.
   * @return адрес HS Daemon-а.
   * @throws AdminException если адрес не задан или InstallationType != HS
   */
  public String getHSDaemonHost() throws AdminException {
    if (getInstallationType() != InstallationType.HS)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getString("host");
    } catch (ConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  /**
   * Возвращает порт HS Daemon-а.
   * @return порт HS Daemon-а.
   * @throws AdminException если порт не задан или InstallationType != HS
   */
  public int getHSDaemonPort() throws AdminException {
    if (getInstallationType() != InstallationType.HS)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection daemon = webconfig.getSection("daemon");
      return daemon.getInt("port");
    } catch (ConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }

  /**
   * Возвращает список хостов, управляемых HS Daemon-ом.
   * @return список хостов, управляемых HS Daemon-ом.
   * @throws AdminException если список не задан или InstallationType != HS
   */
  public Collection<String> getHSDaemonHosts() throws AdminException {
    if (getInstallationType() != InstallationType.HS)
      throw new AdminException("Illegal installation type: " + getInstallationType());

    try {
      XmlConfigSection nodes = webconfig.getSection("nodes");
      Collection<String> result = new ArrayList<String>();
      for (XmlConfigSection nodeSection : nodes.sections())
        result.add(nodeSection.getName());

      return result;
    } catch (ConfigException e) {
      throw new AdminException(e.getMessage(), e);
    }
  }


  // =================================== For HA Installation Type ========================================

  /**
   * Возвращает путь к конфигурационному файлу HA Daemon-а
   *
   * @return путь к конфигурационному файлу HA Daemon-а
   * @throws AdminException если InstallationType != HA
   */
  public File getHAResourceGroupsFile() throws AdminException {
    if (getInstallationType() != InstallationType.HA)
      throw new AdminException("Illegal installation type: " + getInstallationType());
    return new File(appBaseDir, "conf/resourceGroups.properties");
  }


}
