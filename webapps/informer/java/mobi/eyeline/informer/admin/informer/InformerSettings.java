package mobi.eyeline.informer.admin.informer;

import mobi.eyeline.informer.admin.AdminException;
import mobi.eyeline.informer.admin.util.validation.ValidationHelper;

/**
 * Настройки Informer
 *
 * @author Aleksandr Khalitov
 */
public class InformerSettings {

  private final ValidationHelper vh = new ValidationHelper(InformerSettings.class);

  private String adminHost;
  private int adminPort;

  private String dcpHost;
  private int dcpPort;

  private String persHost;
  private int persSyncPort;
  private int persAsyncPort;

  private String statDir;
  private String storeDir;

  public String getAdminHost() {
    return adminHost;
  }

  public void setAdminHost(String host) throws AdminException {
    vh.checkNotEmpty("adminHost", host);
    this.adminHost = host;
  }

  public int getAdminPort() {
    return adminPort;
  }

  public void setAdminPort(int adminPort) throws AdminException {
    vh.checkPort("adminPort", adminPort);
    this.adminPort = adminPort;
  }

  public int getDcpPort() {
    return dcpPort;
  }

  public void setDcpPort(int port) throws AdminException {
    vh.checkPort("dcpPort", port);
    this.dcpPort = port;
  }

  public String getDcpHost() {
    return dcpHost;
  }

  public void setDcpHost(String dcpHost) throws AdminException {
    vh.checkNotEmpty("dcpHost", dcpHost);
    this.dcpHost = dcpHost;
  }

  public String getPersHost() {
    return persHost;
  }

  public void setPersHost(String persHost) throws AdminException {
    vh.checkNotEmpty("persHost", persHost);
    this.persHost = persHost;
  }

  public int getPersSyncPort() {
    return persSyncPort;
  }

  public void setPersSyncPort(int persSyncPort) throws AdminException {
    vh.checkPort("persSyncPort", persSyncPort);
    this.persSyncPort = persSyncPort;
  }

  public int getPersAsyncPort() {
    return persAsyncPort;
  }

  public void setPersAsyncPort(int persAsyncPort) throws AdminException {
    vh.checkPort("persAsyncPort", persAsyncPort);
    this.persAsyncPort = persAsyncPort;
  }

  public String getStatDir() {
    return statDir;
  }

  public void setStatDir(String statDir) throws AdminException {
    vh.checkNotEmpty("statDir", statDir);
    this.statDir = statDir;
  }

  public String getStoreDir() {
    return storeDir;
  }

  public void setStoreDir(String storeDir) throws AdminException {
    vh.checkNotEmpty("storeDir", storeDir);
    this.storeDir = storeDir;
  }

  /**
   * Копирует настройки
   *
   * @return копия настроек
   */
  public InformerSettings cloneSettings() {
    InformerSettings cs = new InformerSettings();
    cs.adminHost = adminHost;
    cs.adminPort = adminPort;
    cs.dcpHost = dcpHost;
    cs.dcpPort = dcpPort;
    cs.persHost = persHost;
    cs.persSyncPort = persSyncPort;
    cs.persAsyncPort = persAsyncPort;
    cs.statDir = statDir;
    cs.storeDir = storeDir;
    return cs;
  }

}
