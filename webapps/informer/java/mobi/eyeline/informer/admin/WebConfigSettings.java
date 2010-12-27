package mobi.eyeline.informer.admin;

import mobi.eyeline.informer.admin.cdr.CdrSettings;
import mobi.eyeline.informer.admin.notifications.NotificationSettings;
import mobi.eyeline.informer.admin.siebel.SiebelSettings;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Properties;

/**
 * Copyright Eyeline.mobi
 * User: vmax
 * Date: 11.11.2010
 * Time: 15:09:49
 */
class WebConfigSettings {
  
  private InstallationType installationType;
  private String daemonHost;
  private Integer daemonPort;
  private File[] appMirrorDirs;
  private Collection<String> HSDaemonHosts;
  private String journalDir;

  private NotificationSettings notificationSettings;  

  private SiebelSettings siebelSettings;
  private CdrSettings cdrSettings;
  private boolean allowUssdPushDeliveries;
  private String workDir;

  private int contentProviderPeriod;

  int getContentProviderPeriod() {
    return contentProviderPeriod;
  }

  void setContentProviderPeriod(int contentProviderPeriod) {
    this.contentProviderPeriod = contentProviderPeriod;
  }


  public String getWorkDir() {
    return workDir;
  }

  public void setWorkDir(String workDir) {
    this.workDir = workDir;
  }

  InstallationType getInstallationType() {
    return installationType;
  }

  void setInstallationType(InstallationType installationType) {
    this.installationType = installationType;
  }


  String getSingleDaemonHost() {
    return daemonHost;
  }

  void setDaemonHost(String daemonHost) {
    this.daemonHost = daemonHost;
  }


  Integer getSingleDaemonPort() {
    return daemonPort;
  }

  void setDaemonPort(Integer daemonPort) {
    this.daemonPort = daemonPort;
  }


  File[] getAppMirrorDirs() {
    File[] ret = new File[appMirrorDirs.length];
    System.arraycopy(appMirrorDirs, 0, ret, 0, ret.length);
    return ret;
  }

  void setAppMirrorDirs(File[] appMirrorDirs) {
    this.appMirrorDirs = appMirrorDirs;
  }

  String getHSDaemonHost() {
    return daemonHost;
  }


  Integer getHSDaemonPort() {
    return daemonPort;
  }


  Collection<String> getHSDaemonHosts() {
    return new ArrayList<String>(HSDaemonHosts);
  }

  void setHSDaemonHosts(Collection<String> HSDaemonHosts) {
    this.HSDaemonHosts = HSDaemonHosts;
  }


  String getJournalDir() {
    return journalDir;
  }

  void setJournalDir(String journalDir) {
    this.journalDir = journalDir;
  }


  NotificationSettings getNotificationSettings() {
    return new NotificationSettings(notificationSettings);
  }

  void setNotificationSettings(NotificationSettings settings) throws AdminException{
    settings.validate();
    this.notificationSettings = new NotificationSettings(settings);
  }

  void setSiebelSettings(SiebelSettings siebelSettings) throws AdminException{
    this.siebelSettings = new SiebelSettings(siebelSettings);
  }

  SiebelSettings getSiebelSettings() {
    return new SiebelSettings(siebelSettings);
  }

  CdrSettings getCdrSettings() {
    return new CdrSettings(cdrSettings);
  }

  void setCdrSettings(CdrSettings cdrSettings) throws AdminException{
    this.cdrSettings = new CdrSettings(cdrSettings);
  }

  boolean isAllowUssdPushDeliveries() {
    return allowUssdPushDeliveries;
  }

  void setAllowUssdPushDeliveries(boolean allowUssdPushDeliveries) {
    this.allowUssdPushDeliveries = allowUssdPushDeliveries;
  }

  private Properties cloneProps(Properties other) {
    Properties props = new Properties();
    for (Object s : other.keySet()) {
      props.put(s, other.get(s));
    }
    return props;
  }
}
